/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * RF-2A4M1 (GenBasic) 2.4 GHz USB Wi-Fi driver - private header.
 *
 * A single, fully-GPL cfg80211 kernel module: the 802.11 MAC/MLME
 * core drives the MT7601U silicon through an in-module HAL vtable, and a
 * kernel USB HAL realizes that vtable over usbcore.  The MAC runs host-side in
 * the module's core (a FullMAC-from-host design that registers a wiphy and a
 * net_device with cfg80211 - the module carries its own MAC and touches no
 * mac80211 ABI).  The only proprietary artifact is the on-chip firmware
 * rf-2a4m1.bin, loaded at runtime via request_firmware().
 *
 * Copyright (C) GenBasic.
 */
#ifndef RF_2A4M1_H
#define RF_2A4M1_H

#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/usb.h>
#include <linux/workqueue.h>
#include <net/cfg80211.h>

/* The MAC/MLME/crypto core (compiled from GPL source into this same .ko).
 * It reaches the kernel only through the HAL vtable declared here. */
#include "rf_2a4m1_core.h"

#define RF_2A4M1_VERSION	"1.0.0"

/* Firmware image, loaded at runtime from /lib/firmware/. */
#define RF_2A4M1_FIRMWARE	"rf-2a4m1/rf-2a4m1.bin"

/* USB bulk endpoints, in enumeration order (6 OUT / 2 IN on the MT7601U USB
 * interface).  Index 0 OUT is the in-band MCU command endpoint; OUT 1..4 are
 * the EDCA access-category queues; IN 0 is the packet-RX endpoint. */
#define RF_2A4M1_N_EP_OUT	6
#define RF_2A4M1_N_EP_IN	2
#define RF_2A4M1_EP_OUT_CMD	0	/* in-band command / firmware DMA */
#define RF_2A4M1_EP_OUT_DATA	2	/* AC_BE data queue */
#define RF_2A4M1_EP_IN_RX	0	/* packet RX */
#define RF_2A4M1_EP_IN_CMD	1	/* MCU command-response */

/* Connect-completion poll (glue-side up-call driver; see cfg80211.c). */
#define RF_2A4M1_CONNECT_POLL_MS	50
/*
 * ~10 s. The window must outlast the WHOLE ladder -- scan, auth, assoc, then
 * the AP's own M1 (which some APs delay well past association) and the 4-way.
 * At 3 s a real AP's association alone could consume the budget, so a timeout
 * reported "stalled at ASSOCED" when the exchange had simply not finished.
 */
#define RF_2A4M1_CONNECT_MAX_POLLS	200

/* RX URB ring depth (bulk-IN pipeline). */
#define RF_2A4M1_RX_URBS	4
/*
 * One bulk-IN buffer must hold a WHOLE aggregate, not one frame: the chip is
 * programmed to pack up to MT_USB_AGGR_SIZE_LIMIT (28) * 1024 B into a single
 * transfer. A buffer smaller than that aggregate overflows the URB (-EOVERFLOW)
 * and loses the transfer, so size it to the limit with headroom, matching the
 * in-tree driver's 32 KB (PAGE_SIZE << MT_RX_ORDER) RX buffers.
 */
#define RF_2A4M1_RX_BUF_SZ	32768
/*
 * TX is sized SEPARATELY from RX and must stay well under KMALLOC_MAX_CACHE_SIZE:
 * rf_2a4m1_op_tx() allocates per-frame in ATOMIC context, where the large-kmalloc
 * path (which forwards to alloc_pages) sleeps and panics. One TX frame is only
 * TXWI + MPDU + DMA framing, so borrowing the RX aggregate size here would not be
 * merely wasteful -- it is unsafe. (The MCU path keeps the RX size deliberately:
 * it also drains the packet-RX endpoint, and it runs at probe in process context.)
 */
#define RF_2A4M1_TX_BUF_SZ	2048

/* Device lifecycle. */
enum rf_2a4m1_state {
	RF_2A4M1_STATE_INIT = 0,
	RF_2A4M1_STATE_FW_LOADED,
	RF_2A4M1_STATE_RUNNING,
	RF_2A4M1_STATE_REMOVING,
};

/* One RX URB + its coherent buffer. */
struct rf_2a4m1_rx_urb {
	struct urb		*urb;
	u8			*buf;
	struct rf_2a4m1_dev	*dev;
};

/* Cached silicon parameters read from the chip EEPROM (efuse) at bring-up:
 * the MAC address plus the per-channel PHY parameters the channel program
 * needs.  Populated by rf_2a4m1_chip_init(); see src/usb.c. */
struct rf_2a4m1_eeprom {
	u8	mac[6];
	u8	rf_freq_off;
	s8	lna_gain;
	s8	ref_temp;
	u8	chan_pwr[14];		/* per-channel target TX power */
	s8	cck_bw20[2];
	s8	ofdm_bw20[4];
	bool	valid;
};

struct rf_2a4m1_dev {
	struct usb_device	*udev;
	struct usb_interface	*intf;
	struct wiphy		*wiphy;
	struct net_device	*ndev;		/* FullMAC: MAC in the core */
	struct wireless_dev	wdev;
	struct device		*dev;
	enum rf_2a4m1_state	state;
	struct mutex		lock;
	struct workqueue_struct	*wq;

	/* Discovered bulk-endpoint addresses (bEndpointAddress). */
	u8			out_eps[RF_2A4M1_N_EP_OUT];
	u8			in_eps[RF_2A4M1_N_EP_IN];
	int			n_out, n_in;

	/* The in-module HAL: the core calls hal.ops->* (installed by src/usb.c);
	 * the kernel USB HAL implements them over usbcore. */
	struct rf_2a4m1_hal	hal;

	/* The SME/MLME instance the cfg80211 ops drive (the MAC runs
	 * host-side in this module's core). */
	struct rf_2a4m1_sme	sme;

	/* RX URB ring. */
	struct rf_2a4m1_rx_urb	rx[RF_2A4M1_RX_URBS];
	/*
	 * In-flight async TX URBs. Data TX is submitted from the core's RX
	 * handler, so it cannot use the synchronous usb_bulk_msg(); the anchor
	 * is what lets teardown wait for the fire-and-forget URBs, without which
	 * one completing after the module unloads would call into freed code.
	 */
	struct usb_anchor	tx_anchor;
	/*
	 * RX hand-off to PROCESS context.
	 *
	 * A URB completes in softirq, but the core answers a received frame by
	 * calling straight back down into the HAL (set_lower_mac, reg_read, tx
	 * ...), and those do synchronous USB control/bulk transfers that SLEEP.
	 * Delivering from the completion handler is therefore a "sleeping
	 * function called from invalid context" BUG -- one that only fires once
	 * a real AP answers, because that is the first time the core has any
	 * reason to talk back. Queue the segments here and let a work item
	 * deliver them where sleeping is legal.
	 */
	struct sk_buff_head	rx_queue;
	struct work_struct	rx_work;

	rf_2a4m1_mac_addr	macaddr;

	/* Chip bring-up state (src/usb.c). */
	struct rf_2a4m1_eeprom	ee;
	bool			hw_inited;
	int			cur_channel;
	u8			mcu_seq;	/* MCU command-response sequence */

	/* Connect-completion up-call driver (src/cfg80211.c). */
	struct delayed_work	connect_work;
	u8			connect_bssid[RF_2A4M1_ETH_ALEN];
	unsigned int		connect_polls;
	bool			connect_pending;

	/*
	 * Connect target's SSID + channel, captured from the connect request so
	 * the connect worker can publish the AP's BSS to cfg80211 BEFORE it
	 * reports a successful result. cfg80211 warns on a successful connect
	 * for a BSS it was never told about (net/wireless/sme.c), and it matches
	 * that BSS on BSSID + SSID -- so both must be on hand at completion time.
	 */
	u8			connect_ssid[IEEE80211_MAX_SSID_LEN];
	u8			connect_ssid_len;
	struct ieee80211_channel *connect_chan;

	/*
	 * Latest beacon / probe-response overheard FROM the connect target, so
	 * the completion can inform cfg80211 with the AP's own IEs (the faithful
	 * cfg80211_inform_bss_frame path) instead of a constructed stub. Written
	 * by the RX worker, read by the connect worker -- two work items that can
	 * run on different CPUs, so the spinlock makes the hand-off race-free.
	 * A fixed buffer (no allocation lifecycle to leak): a frame that does not
	 * fit is simply not captured and the worker builds a minimal BSS.
	 */
	spinlock_t		bss_frame_lock;
	u16			bss_frame_len;	/* 0 = none captured */
	s8			bss_frame_rssi;	/* dBm, as the RXWI delivered it */
	u8			bss_frame[512];

	/*
	 * RX instrumentation. ndev->stats.rx_packets only counts DATA frames
	 * bridged to the stack, so it is 0 before association and cannot tell
	 * "the chip hears nothing" apart from "frames arrive but the SME
	 * rejects them" -- which are opposite root causes for a SCANNING stall.
	 */
	atomic_t		rx_urbs;	/* RX URB completions (status==0) */
	atomic_t		rx_frames;	/* frames parsed + delivered up   */
	/*
	 * A bulk-IN transfer carries an AGGREGATE of DMA segments, not one frame
	 * (the chip is told to aggregate; see MT_USB_DMA_CFG_RX_BULK_AGG_EN).
	 * rx_frames alone therefore cannot tell "the air is quiet" apart from
	 * "the air is busy and we are dropping all but the first frame of every
	 * transfer" -- opposite root causes for a starved ring. Count the
	 * segments the chip actually delivered against the frames we consumed.
	 */
	atomic_t		rx_segs;	/* DMA segments present in the ring */
	atomic_t		rx_seg_max;	/* most segments in one transfer    */
	/*
	 * A URB that completes with an error is NOT resubmitted, so each one
	 * permanently shrinks the ring: RF_2A4M1_RX_URBS such errors and RX is
	 * dead for good. That is invisible in rx_urbs (which only counts
	 * successes), so count the failures and keep the last status.
	 */
	atomic_t		rx_urb_errs;	/* completions with status != 0     */
	atomic_t		rx_urb_last_err;/* the last such status (-errno)    */
	atomic_t		rx_bcn_logged;	/* bounds the per-beacon source log */
	/*
	 * EAPOL-Key frames seen on the wire. The 4-way rides DATA frames, so the
	 * mgmt-subtype histogram is blind to it: without this, "the AP never sent
	 * M1" and "M1 arrived and the core ignored it" look identical.
	 */
	atomic_t		rx_eapol;
	atomic_t		rx_type[4];	/* 0=mgmt 1=ctrl 2=data 3=ext  */
	atomic_t		rx_data_to_us;	/* data frames with RA == our MAC */
	atomic_t		rx_d2u_logged;	/* bounds the data-to-us dump      */
	u8			connect_last_state;	/* FSM step tracing */
	/* Mgmt frames by 802.11 subtype: 8=beacon 5=probe-resp 11=auth
	 * 1=assoc-resp. The SME leaves SCANNING only on a PROBE_RESP and has no
	 * beacon frame type at all, so this separates "the AP never answered our
	 * probe" from "it answered and the SME rejected it". */
	atomic_t		rx_mgmt_sub[16];
	/* Probe-responses whose SA is the BSSID we were asked to connect to.
	 * rx_mgmt_sub[5] counts probe-responses from EVERY AP in the cell, so it
	 * cannot answer "did the TARGET answer us?" -- which is the question when
	 * a connect stalls in scan amid neighbouring APs. */
	atomic_t		rx_proberesp_target;
	atomic_t		rx_auth_target;
	/*
	 * TX instrumentation. Deliberately NOT an 802.11-subtype histogram: the
	 * SME hands the HAL its own short internal frame, so decoding byte 0 as a
	 * Frame Control field yields a plausible but meaningless subtype. Count
	 * frames and record the length instead -- a valid 802.11 management frame
	 * is >= 24 B of header alone, so a short MPDU here means we are radiating
	 * something no AP can parse.
	 */
	atomic_t		tx_calls;	/* every frame handed to the radio */
	atomic_t		tx_last_len;	/* length of the last MPDU radiated */
};

/*
 * Device lookup from a wiphy / net_device.
 *
 * Both objects are allocated with priv_size = sizeof(struct rf_2a4m1_dev *), so
 * their priv slot holds a POINTER to the separately-allocated device rather than
 * the device itself (see rf_2a4m1_wiphy_alloc() + rf_2a4m1_cfg80211_register()).
 *
 * ALWAYS use these accessors and never open-code the conversion: wiphy_priv() and
 * netdev_priv() return void *, so omitting the extra dereference is type-legal C
 * that silently yields a pointer to the priv slot itself -- every subsequent
 * dev->... then reads past the allocation. No compiler or static checker can
 * catch that; keeping the indirection in these four helpers is what prevents it.
 */
static inline struct rf_2a4m1_dev *rf_2a4m1_from_wiphy(struct wiphy *wiphy)
{
	return *(struct rf_2a4m1_dev **)wiphy_priv(wiphy);
}

static inline struct rf_2a4m1_dev *rf_2a4m1_from_ndev(struct net_device *ndev)
{
	return *(struct rf_2a4m1_dev **)netdev_priv(ndev);
}

static inline void rf_2a4m1_set_wiphy_dev(struct wiphy *wiphy,
					  struct rf_2a4m1_dev *dev)
{
	*(struct rf_2a4m1_dev **)wiphy_priv(wiphy) = dev;
}

static inline void rf_2a4m1_set_ndev_dev(struct net_device *ndev,
					 struct rf_2a4m1_dev *dev)
{
	*(struct rf_2a4m1_dev **)netdev_priv(ndev) = dev;
}

/* --- src/usb.c (kernel USB HAL) --- */
int rf_2a4m1_usb_probe_setup(struct rf_2a4m1_dev *dev);
int rf_2a4m1_usb_load_firmware(struct rf_2a4m1_dev *dev,
			       const u8 *fw, size_t fw_len);
void rf_2a4m1_usb_hal_init(struct rf_2a4m1_dev *dev);
int rf_2a4m1_usb_rx_start(struct rf_2a4m1_dev *dev);
void rf_2a4m1_usb_rx_stop(struct rf_2a4m1_dev *dev);

/* MAC/BBP/RF chip bring-up (the register init sequence run once after the MCU
 * firmware boots; reads the EEPROM MAC into dev->macaddr + dev->ee). */
int rf_2a4m1_chip_init(struct rf_2a4m1_dev *dev, int channel);

/* register access (over usb_control_msg) - exposed for the fw-load path. */
u32 rf_2a4m1_reg_read(struct rf_2a4m1_dev *dev, u16 offset);
int rf_2a4m1_reg_write(struct rf_2a4m1_dev *dev, u16 offset, u32 val);

/* --- src/cfg80211.c --- */
int rf_2a4m1_cfg80211_register(struct rf_2a4m1_dev *dev);
void rf_2a4m1_cfg80211_unregister(struct rf_2a4m1_dev *dev);
struct wiphy *rf_2a4m1_wiphy_alloc(struct device *dev);

#endif /* RF_2A4M1_H */
