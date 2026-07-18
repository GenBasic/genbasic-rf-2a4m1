// SPDX-License-Identifier: GPL-2.0-only
/*
 * RF-2A4M1 - cfg80211 interface.
 *
 * Registers a wiphy + net_device with cfg80211 and advertises the MT7601U
 * silicon envelope (2.4 GHz, channels 1-13, 802.11b/g rates, HT MCS 0-7 1SS,
 * WEP/TKIP/CCMP/BIP ciphers, STA/AP/monitor).  The cfg80211 ops marshal user
 * requests into the MAC/MLME core's SME entry symbols (the MAC
 * runs host-side in this module's core), and the core drives the silicon
 * through the kernel USB HAL (src/usb.c).
 *
 * Copyright (C) GenBasic.
 */
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/random.h>
#include <linux/string.h>
#include <net/cfg80211.h>

#include "rf-2a4m1.h"

/* ---- 2.4 GHz channels 1-13 (channel 14 / 2484 MHz is JP-only, disabled). ---- */
#define CHAN2G(_ch, _freq) {			\
	.band = NL80211_BAND_2GHZ,		\
	.center_freq = (_freq),			\
	.hw_value = (_ch),			\
	.max_power = 20,			\
}
static struct ieee80211_channel rf_2a4m1_channels[] = {
	CHAN2G(1, 2412), CHAN2G(2, 2417), CHAN2G(3, 2422), CHAN2G(4, 2427),
	CHAN2G(5, 2432), CHAN2G(6, 2437), CHAN2G(7, 2442), CHAN2G(8, 2447),
	CHAN2G(9, 2452), CHAN2G(10, 2457), CHAN2G(11, 2462), CHAN2G(12, 2467),
	CHAN2G(13, 2472),
};

/* ---- 802.11b (CCK) + 802.11g (OFDM) bitrates. ---- */
#define RATE(_rate, _flags) {			\
	.bitrate = (_rate),			\
	.hw_value = (_rate),			\
	.flags = (_flags),			\
}
static struct ieee80211_rate rf_2a4m1_rates[] = {
	RATE(10, 0), RATE(20, IEEE80211_RATE_SHORT_PREAMBLE),
	RATE(55, IEEE80211_RATE_SHORT_PREAMBLE),
	RATE(110, IEEE80211_RATE_SHORT_PREAMBLE),
	RATE(60, 0), RATE(90, 0), RATE(120, 0), RATE(180, 0),
	RATE(240, 0), RATE(360, 0), RATE(480, 0), RATE(540, 0),
};

/* HT: HT20/HT40, SGI-20/40, 1-stream MCS 0-7. */
static struct ieee80211_supported_band rf_2a4m1_band_2ghz = {
	.band		= NL80211_BAND_2GHZ,
	.channels	= rf_2a4m1_channels,
	.n_channels	= ARRAY_SIZE(rf_2a4m1_channels),
	.bitrates	= rf_2a4m1_rates,
	.n_bitrates	= ARRAY_SIZE(rf_2a4m1_rates),
	.ht_cap = {
		.ht_supported = true,
		.cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 |
		       IEEE80211_HT_CAP_SGI_20 |
		       IEEE80211_HT_CAP_SGI_40 |
		       IEEE80211_HT_CAP_RX_STBC,
		.ampdu_factor = IEEE80211_HT_MAX_AMPDU_64K,
		.ampdu_density = IEEE80211_HT_MPDU_DENSITY_4,
		.mcs = {
			.rx_mask = { 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			.rx_highest = cpu_to_le16(72),	/* MCS7 HT20 SGI */
			.tx_params = IEEE80211_HT_MCS_TX_DEFINED,
		},
	},
};

static const u32 rf_2a4m1_cipher_suites[] = {
	WLAN_CIPHER_SUITE_WEP40,
	WLAN_CIPHER_SUITE_WEP104,
	WLAN_CIPHER_SUITE_TKIP,
	WLAN_CIPHER_SUITE_CCMP,
	WLAN_CIPHER_SUITE_AES_CMAC,	/* BIP-CMAC-128 (PMF / 802.11w) */
};

/* ================================================================== */
/* The SME crypto vtable the 4-way needs, bound to the        */
/* core's real WPA2 primitives.                                       */
/*                                                                    */
/*   .gen_nonce  -> kernel CSPRNG (get_random_bytes)                   */
/*   .kdf        -> rf_2a4m1_wpa_derive_ptk (PRF-384 pairwise-key exp) */
/*   .mic        -> rf_2a4m1_hmac_sha1, truncated to 16 B (AKM 2 MIC)  */
/*   .key_wrap   -> rf_2a4m1_aes_key_wrap   (NIST AES Key Wrap 3394)   */
/*   .key_unwrap -> rf_2a4m1_aes_key_unwrap                            */
/*                                                                    */
/* So the 4-way now derives a real PTK/GTK from the PMK, the nonces,   */
/* and the peer addresses, and computes/verifies real EAPOL-Key MICs.  */
/*                                                                    */
/* The SME now exchanges the full 32-octet 802.11 Key Nonce on the     */
/* wire (RF_2A4M1_SME_NONCE_LEN == RF_2A4M1_WPA_NONCE_LEN == 32), so    */
/* the .kdf adapter binds rf_2a4m1_wpa_derive_ptk directly - no nonce   */
/* zero-extend - giving a standards-exact 4-way against a real          */
/* third-party AP.                                                     */
/* ================================================================== */

/* KCK/TK are the 16-B halves of the 48-B PTK; the EAPOL-Key/data MIC key. */
#define RF_2A4M1_MIC_KEY_LEN	16

static void rf_2a4m1_crypto_gen_nonce(u8 out[RF_2A4M1_SME_NONCE_LEN],
				      const u8 *pmk,
				      const rf_2a4m1_mac_addr *mac,
				      u64 salt)
{
	(void)pmk; (void)mac; (void)salt;	/* real entropy: salt unused */
	get_random_bytes(out, RF_2A4M1_SME_NONCE_LEN);
}

static void rf_2a4m1_crypto_kdf(u8 ptk[SME_PTK_LEN], const u8 *pmk,
				const u8 anonce[RF_2A4M1_SME_NONCE_LEN],
				const u8 snonce[RF_2A4M1_SME_NONCE_LEN],
				const rf_2a4m1_mac_addr *a,
				const rf_2a4m1_mac_addr *b)
{
	/*
	 * The SME now exchanges the full 32-octet 802.11 Key Nonce on the wire
	 * (RF_2A4M1_SME_NONCE_LEN == RF_2A4M1_WPA_NONCE_LEN), so the PRF-384 nonce
	 * inputs pass straight through -- no 8->32 zero-extend pad.  PRF-384 sorts
	 * Min/Max(AA,SPA) and Min/Max(nonce) internally, so the a/b (self/peer)
	 * order passed here is immaterial to the derived key.
	 */
	rf_2a4m1_wpa_derive_ptk(pmk, a, b, anonce, snonce, ptk);
}

static void rf_2a4m1_crypto_mic(u8 out[RF_2A4M1_SME_MIC_LEN], const u8 *kck,
				const u8 *frame, size_t len)
{
	u8 digest[RF_2A4M1_SHA1_DIGEST_LEN];

	/* WPA2 (AKM 2) EAPOL-Key MIC = HMAC-SHA1-128: first 16 B of HMAC-SHA1. */
	rf_2a4m1_hmac_sha1(kck, RF_2A4M1_MIC_KEY_LEN, frame, len, digest);
	memcpy(out, digest, RF_2A4M1_SME_MIC_LEN);
}

static size_t rf_2a4m1_crypto_key_wrap(u8 *out, size_t out_cap, const u8 *kek,
				       const u8 *in, size_t in_len)
{
	return rf_2a4m1_aes_key_wrap(kek, in, in_len, out, out_cap);
}

static size_t rf_2a4m1_crypto_key_unwrap(u8 *out, size_t out_cap, const u8 *kek,
					 const u8 *in, size_t in_len)
{
	return rf_2a4m1_aes_key_unwrap(kek, in, in_len, out, out_cap);
}

static const struct rf_2a4m1_sme_crypto rf_2a4m1_sme_crypto_impl = {
	.gen_nonce  = rf_2a4m1_crypto_gen_nonce,
	.kdf	    = rf_2a4m1_crypto_kdf,
	.mic	    = rf_2a4m1_crypto_mic,
	.key_wrap   = rf_2a4m1_crypto_key_wrap,
	.key_unwrap = rf_2a4m1_crypto_key_unwrap,
};

/* ================================================================== */
/* netdev ops (FullMAC data path).                                    */
/* ================================================================== */
static int rf_2a4m1_ndo_open(struct net_device *ndev)
{
	netif_start_queue(ndev);
	return 0;
}

static int rf_2a4m1_ndo_stop(struct net_device *ndev)
{
	netif_stop_queue(ndev);
	return 0;
}

static netdev_tx_t rf_2a4m1_ndo_start_xmit(struct sk_buff *skb,
					   struct net_device *ndev)
{
	struct rf_2a4m1_dev *dev = rf_2a4m1_from_ndev(ndev);
	int ret;

	/*
	 * Encapsulate the 802.3 frame as an HW-CCMP-encrypted 802.11 Data frame
	 * and DMA it out on AC_BE: the chip inserts the CCMP header + encrypts +
	 * appends the MIC (native offload, wcid = the pairwise key slot, WIV=0).
	 * Each skb is an independent async bulk-out URB anchored for teardown, so
	 * multiple frames pipeline; the queue is never stopped, so the stack is
	 * free to hand us the next skb immediately.
	 */
	ret = rf_2a4m1_usb_hw_data_tx(dev, skb->data, skb->len);
	if (!ret) {
		ndev->stats.tx_packets++;
		ndev->stats.tx_bytes += skb->len;
	} else {
		ndev->stats.tx_dropped++;
	}
	dev_kfree_skb(skb);
	return NETDEV_TX_OK;
}

static const struct net_device_ops rf_2a4m1_netdev_ops = {
	.ndo_open	= rf_2a4m1_ndo_open,
	.ndo_stop	= rf_2a4m1_ndo_stop,
	.ndo_start_xmit	= rf_2a4m1_ndo_start_xmit,
	.ndo_set_mac_address = eth_mac_addr,
};

/*
 * Publish the connect target's BSS to cfg80211 and return a REFERENCED
 * struct cfg80211_bss for it. The caller hands that reference straight to
 * cfg80211_connect_bss(), which consumes it -- so this must NOT be paired with
 * a cfg80211_put_bss() on the success path.
 *
 * Why it exists: cfg80211 warns on a successful connect result for a BSS it was
 * never told about (net/wireless/sme.c looks the AP up by BSSID + SSID and warns
 * if it is absent). A FullMAC driver is expected to announce the AP first -- via
 * a scan or an inform_bss. This driver has no scan (the .scan op is an
 * unimplemented seam), so it announces the AP here, at connect completion:
 *
 *  - Faithful path: if the RX worker overheard the target's beacon or
 *    probe-response, hand cfg80211 that raw frame + its signal. cfg80211 then
 *    parses the AP's own IEs (SSID, RSN, supported rates, HT) -- exactly what
 *    user space (NetworkManager / wpa_supplicant) reads back off the BSS.
 *  - Fallback: build a minimal BSS (BSSID + SSID IE + ESS/Privacy
 *    capability) from the connect request when no frame was captured.
 *
 * Returns NULL only if a channel cannot be resolved or the inform allocation
 * fails; the caller then falls back to a bssid-only result.
 */
static struct cfg80211_bss *rf_2a4m1_publish_target_bss(struct rf_2a4m1_dev *dev)
{
	struct ieee80211_channel *chan = dev->connect_chan;
	struct cfg80211_bss *bss = NULL;
	u8 frame[sizeof(dev->bss_frame)];
	u8 ie[2 + IEEE80211_MAX_SSID_LEN];
	u16 frame_len;
	size_t ielen;
	s8 rssi;

	/* The connect request usually carries the channel; if not, derive it
	 * from the channel the chip is actually tuned to. */
	if (!chan && dev->cur_channel)
		chan = ieee80211_get_channel(dev->wiphy,
			ieee80211_channel_to_frequency(dev->cur_channel,
						       NL80211_BAND_2GHZ));
	if (!chan)
		return NULL;		/* cannot inform cfg80211 without a channel */

	/* Snapshot whatever the RX worker captured, under the lock. */
	spin_lock(&dev->bss_frame_lock);
	frame_len = dev->bss_frame_len;
	rssi = dev->bss_frame_rssi;
	if (frame_len && frame_len <= sizeof(frame))
		memcpy(frame, dev->bss_frame, frame_len);
	else
		frame_len = 0;
	spin_unlock(&dev->bss_frame_lock);

	/* Signal is mBm when the wiphy's signal type is CFG80211_SIGNAL_TYPE_MBM
	 * (set in rf_2a4m1_wiphy_alloc): dBm * 100. */
	if (frame_len) {
		bss = cfg80211_inform_bss_frame(dev->wiphy, chan,
						(struct ieee80211_mgmt *)frame,
						frame_len, (s32)rssi * 100,
						GFP_KERNEL);
		if (bss)
			return bss;	/* faithful path succeeded */
		/* else: inform failed -- fall through to the constructed BSS */
	}

	ie[0] = WLAN_EID_SSID;
	ie[1] = dev->connect_ssid_len;
	if (dev->connect_ssid_len)
		memcpy(ie + 2, dev->connect_ssid, dev->connect_ssid_len);
	ielen = 2 + dev->connect_ssid_len;

	return cfg80211_inform_bss(dev->wiphy, chan, CFG80211_BSS_FTYPE_UNKNOWN,
				   dev->connect_bssid, 0,
				   WLAN_CAPABILITY_ESS | WLAN_CAPABILITY_PRIVACY,
				   100 /* beacon interval (TU) */,
				   ie, ielen, (s32)rssi * 100, GFP_KERNEL);
}

/* ================================================================== */
/* Connect-completion: the core advances the connect FSM as    */
/* the AP's frames arrive (via the HAL RX -> sme_rx path); it has no    */
/* CONNECTED callback, so a glue-side delayed work polls the SME state  */
/* and fires the cfg80211 up-call when it reaches CONNECTED / FAILED,   */
/* or times out.                                                      */
/* ================================================================== */
static void rf_2a4m1_connect_worker(struct work_struct *w)
{
	struct rf_2a4m1_dev *dev =
		container_of(to_delayed_work(w), struct rf_2a4m1_dev, connect_work);

	if (dev->state == RF_2A4M1_STATE_REMOVING || !dev->connect_pending)
		return;

	/*
	 * Trace every FSM step. The end-of-run state alone cannot separate "the
	 * ladder stopped here" from "the ladder never got here before the poll
	 * window closed" -- and those need opposite fixes.
	 */
	if (dev->sme.state != dev->connect_last_state) {
		dev_info(dev->dev,
			 "connect: sme.state %u -> %u at %u ms (eapol_rx=%d ptk=%d gtk=%d)\n",
			 dev->connect_last_state, dev->sme.state,
			 dev->connect_polls * RF_2A4M1_CONNECT_POLL_MS,
			 atomic_read(&dev->rx_eapol),
			 dev->sme.ptk_installed, dev->sme.gtk_installed);
		dev_info(dev->dev,
			 "connect: 4-way stages: m1_rx=%u m2_tx=%u m3_rx=%u m4_tx=%u bad_mic=%u ocv_fail=%u\n",
			 dev->sme.eapol_m1_rx, dev->sme.eapol_m2_tx,
			 dev->sme.eapol_m3_rx, dev->sme.eapol_m4_tx,
			 dev->sme.bad_mic, dev->sme.ocv_fail);
		dev->connect_last_state = dev->sme.state;
	}

	if (dev->sme.state == RF_2A4M1_SME_CONNECTED || dev->sme.connected) {
		struct cfg80211_bss *bss;

		dev->connect_pending = false;
		/*
		 * The 4-way is done and the SME derived the PTK/GTK.  Program the
		 * MT7601U HW crypto key table from those keys (pairwise TK + GTK +
		 * BSSID) so the data plane is HW-encrypted/decrypted -- the SME ran
		 * the handshake itself, so this is the equivalent of the
		 * add_key/install a wpa_supplicant path would drive.  Process
		 * context (delayed work), so the register writes may sleep.
		 */
		if (!dev->hw_key_installed)
			rf_2a4m1_usb_install_hw_keys(dev);
		/*
		 * Falsifiable control: with the HW key now installed, fire the
		 * bogus-BSSID probe (a few HW-encrypted frames to an address no
		 * peer owns).  Their TX-status SUCCESS must be 0, proving a
		 * SUCCESS=1 on the real encrypted/cleartext path is a genuine ACK
		 * and not a stuck bit.  One-shot (guarded inside).
		 */
		rf_2a4m1_usb_bogus_bssid_probe(dev);
		/*
		 * Publish the AP's BSS to cfg80211 BEFORE reporting success and
		 * pass the referenced bss straight into cfg80211_connect_bss(),
		 * which consumes the reference (so it is NOT put here). With the
		 * bss supplied, cfg80211 skips its own BSSID+SSID lookup and the
		 * bss_not_found WARN (net/wireless/sme.c) it would otherwise hit.
		 * bss == NULL only on OOM: the call then degrades to a bssid-only
		 * result (the pre-fix behaviour), which is the right failure mode.
		 */
		bss = rf_2a4m1_publish_target_bss(dev);
		cfg80211_connect_bss(dev->ndev, dev->connect_bssid, bss, NULL, 0,
				     NULL, 0, WLAN_STATUS_SUCCESS, GFP_KERNEL,
				     NL80211_TIMEOUT_UNSPECIFIED);
		return;
	}
	if (dev->sme.state == RF_2A4M1_SME_FAILED) {
		dev->connect_pending = false;
		cfg80211_connect_result(dev->ndev, dev->connect_bssid, NULL, 0,
					NULL, 0, WLAN_STATUS_UNSPECIFIED_FAILURE,
					GFP_KERNEL);
		return;
	}
	if (++dev->connect_polls > RF_2A4M1_CONNECT_MAX_POLLS) {
		dev->connect_pending = false;
		/*
		 * Report WHERE the FSM stalled. Without this the timeout is
		 * indistinguishable from "nothing happened": state names are
		 * 0=INIT 1=SCANNING 2=AUTHED 3=ASSOCED 4=4WAY 5=CONNECTED
		 * 6=FAILED. rx_packets tells TX-side vs RX-side failure apart.
		 */
		dev_info(dev->dev,
			 "connect: TIMEOUT after %u polls (%u ms); sme.state=%u connected=%d ptk=%d gtk=%d rx_urbs=%d rx_frames=%d rx_packets=%lu rx_errors=%lu rx_dropped=%lu\n",
			 dev->connect_polls,
			 dev->connect_polls * RF_2A4M1_CONNECT_POLL_MS,
			 dev->sme.state, dev->sme.connected,
			 dev->sme.ptk_installed, dev->sme.gtk_installed,
			 atomic_read(&dev->rx_urbs), atomic_read(&dev->rx_frames),
			 dev->ndev->stats.rx_packets, dev->ndev->stats.rx_errors,
			 dev->ndev->stats.rx_dropped);
		dev_info(dev->dev,
			 "connect: rx mgmt by subtype: beacon=%d probe_resp=%d auth=%d assoc_resp=%d probe_req=%d deauth=%d\n",
			 atomic_read(&dev->rx_mgmt_sub[8]),
			 atomic_read(&dev->rx_mgmt_sub[5]),
			 atomic_read(&dev->rx_mgmt_sub[11]),
			 atomic_read(&dev->rx_mgmt_sub[1]),
			 atomic_read(&dev->rx_mgmt_sub[4]),
			 atomic_read(&dev->rx_mgmt_sub[12]));
		dev_info(dev->dev,
			 "connect: TARGET %pM: probe_resp=%d auth=%d\n",
			 dev->connect_bssid,
			 atomic_read(&dev->rx_proberesp_target),
			 atomic_read(&dev->rx_auth_target));
		dev_info(dev->dev,
			 "connect: tx_calls=%d last_mpdu_len=%d (a valid 802.11 mgmt frame is >=24 B of header alone)\n",
			 atomic_read(&dev->tx_calls), atomic_read(&dev->tx_last_len));
		dev_info(dev->dev,
			 "connect: 4-way stages (final): m1_rx=%u m2_tx=%u m3_rx=%u m4_tx=%u bad_mic=%u ocv_fail=%u\n",
			 dev->sme.eapol_m1_rx, dev->sme.eapol_m2_tx,
			 dev->sme.eapol_m3_rx, dev->sme.eapol_m4_tx,
			 dev->sme.bad_mic, dev->sme.ocv_fail);
		/*
		 * The ring itself. segs>frames means the chip delivered frames we
		 * threw away; urb_errs>0 means the ring is being dismantled one
		 * URB at a time (RF_2A4M1_RX_URBS of them and RX is dead).
		 */
		dev_info(dev->dev,
			 "connect: rx by type: mgmt=%d ctrl=%d data=%d (data_to_us=%d eapol_rx=%d) -- data=0 means the MAC is not admitting data frames; data>0 with eapol_rx=0 means the AP never sent M1\n",
			 atomic_read(&dev->rx_type[0]),
			 atomic_read(&dev->rx_type[1]),
			 atomic_read(&dev->rx_type[2]),
			 atomic_read(&dev->rx_data_to_us),
			 atomic_read(&dev->rx_eapol));
		dev_info(dev->dev,
			 "connect: rx ring: segs=%d seg_max=%d urb_errs=%d last_err=%d (urbs=%d frames=%d of %d urbs x %d B)\n",
			 atomic_read(&dev->rx_segs), atomic_read(&dev->rx_seg_max),
			 atomic_read(&dev->rx_urb_errs),
			 atomic_read(&dev->rx_urb_last_err),
			 atomic_read(&dev->rx_urbs), atomic_read(&dev->rx_frames),
			 RF_2A4M1_RX_URBS, RF_2A4M1_RX_BUF_SZ);
		cfg80211_connect_timeout(dev->ndev, dev->connect_bssid, NULL, 0,
					 GFP_KERNEL, NL80211_TIMEOUT_UNSPECIFIED);
		return;
	}
	schedule_delayed_work(&dev->connect_work,
			      msecs_to_jiffies(RF_2A4M1_CONNECT_POLL_MS));
}

/* ================================================================== */
/* cfg80211 ops.                                                      */
/* ================================================================== */
static int rf_2a4m1_cfg_scan(struct wiphy *wiphy,
			     struct cfg80211_scan_request *req)
{
	/*
	 * TODO: scan orchestration.  The core has no single
	 * "run a scan" entry (probe-request TX + beacon/probe-resp collection
	 * feeding cfg80211_inform_bss_data + cfg80211_scan_done); it is an
	 * unresolved seam in the host-core<->cfg80211 model (2-dkms-driver.md
	 * D-seam).  Reported, not faked.
	 */
	return -EOPNOTSUPP;
}

/*
 * Optional WPA2 passphrase for the in-driver PSK->PMK derivation.
 *
 * Empty by default: the standard cfg80211 4-way-offload path supplies the
 * pre-derived 32-B PMK in conn->crypto.psk (user space already ran PBKDF2 over
 * passphrase+SSID), which rf_2a4m1_connect_derive_pmk() uses directly.  When a
 * raw passphrase IS set here, the PMK is instead derived in-driver with the
 * PBKDF2 primitive: PMK = PBKDF2-HMAC-SHA1(passphrase, SSID, 4096, 32),
 * the IEEE 802.11i WPA2-PSK derivation.
 */
static char *rf_2a4m1_psk_passphrase;
module_param_named(passphrase, rf_2a4m1_psk_passphrase, charp, 0644);
MODULE_PARM_DESC(passphrase,
	"optional WPA2 passphrase; when set, PMK = PBKDF2-HMAC-SHA1(passphrase, SSID, 4096, 32)");

/*
 * Resolve the WPA2 PMK the 4-way handshake needs from the connect request.
 * Returns true and fills pmk[] on success; false for an open network (no PMK).
 */
static bool rf_2a4m1_connect_derive_pmk(const struct cfg80211_connect_params *conn,
					u8 pmk[RF_2A4M1_SME_PMK_LEN])
{
	/*
	 * (1) Raw-passphrase path (module param): derive the PMK in-driver with
	 *     the PBKDF2 primitive -- PMK = PBKDF2-HMAC-SHA1(passphrase,
	 *     SSID, 4096, 32), the IEEE 802.11i WPA2-PSK derivation.  Takes
	 *     precedence when a passphrase + SSID are present.
	 */
	if (rf_2a4m1_psk_passphrase && rf_2a4m1_psk_passphrase[0] &&
	    conn->ssid && conn->ssid_len) {
		rf_2a4m1_pbkdf2_hmac_sha1((const u8 *)rf_2a4m1_psk_passphrase,
					  strlen(rf_2a4m1_psk_passphrase),
					  conn->ssid, conn->ssid_len, 4096,
					  pmk, RF_2A4M1_SME_PMK_LEN);
		return true;
	}
	/*
	 * (2) Standard cfg80211 4-way-offload path: crypto.psk IS the 32-B WPA2
	 *     PMK (user space already ran PBKDF2 over passphrase+SSID).  Use it
	 *     directly.  cfg80211's connect params carry no WPA2 *raw* passphrase
	 *     field (the only passphrase field, crypto.sae_pwd, is WPA3-SAE, which
	 *     derives its PMK via Dragonfly, not PBKDF2), so this pre-derived key
	 *     is the source on the standard path; the module param above is the
	 *     raw-passphrase alternative.  TODO: a private attr/ioctl to
	 *     carry a WPA2 raw passphrase from user space.
	 */
	if (conn->crypto.psk) {
		memcpy(pmk, conn->crypto.psk, RF_2A4M1_SME_PMK_LEN);
		return true;
	}
	return false;			/* open network: no PMK (no 4-way) */
}

static int rf_2a4m1_cfg_connect(struct wiphy *wiphy, struct net_device *ndev,
				struct cfg80211_connect_params *conn)
{
	struct rf_2a4m1_dev *dev = rf_2a4m1_from_wiphy(wiphy);
	rf_2a4m1_mac_addr peer;
	u8 pmk[RF_2A4M1_SME_PMK_LEN] = { 0 };
	bool have_pmk;

	if (!conn->bssid)
		return -EINVAL;
	memcpy(peer.a, conn->bssid, RF_2A4M1_ETH_ALEN);

	/*
	 * Tune the chip to the target BSS's channel BEFORE starting the SME.
	 * Chip init leaves the radio on its default channel, and nothing else in
	 * the connect path programs it -- so without this the STA transmits auth
	 * on the wrong channel, the AP (on another) never hears it, and the
	 * connect just times out with the AP logging nothing at all.
	 */
	if (conn->channel) {
		struct rf_2a4m1_chan_def cd = {
			.band       = RF_2A4M1_HAL_BAND_2G4,
			.center_mhz = conn->channel->center_freq,
			.width_mhz  = 20,
		};
		int cret = dev->hal.ops->set_channel(&dev->hal, &cd);

		if (cret < 0) {
			dev_err(dev->dev, "connect: set_channel %u MHz failed (%d)\n",
				conn->channel->center_freq, cret);
			return cret;
		}
		dev_info(dev->dev, "connect: tuned to %u MHz (channel %d)\n",
			 conn->channel->center_freq, dev->cur_channel);
	}

	/* Derive the real WPA2 PMK from the connect request (replaces the former
	 * zero PMK) so the 4-way computes real, secret PTK/GTK session keys. */
	have_pmk = rf_2a4m1_connect_derive_pmk(conn, pmk);

	/*
	 * Initialize the SME with the REAL crypto vtable (rf_2a4m1_sme_crypto_impl
	 * - the 4-way derives real keys) seeded with the derived PMK, bind the
	 * kernel USB HAL (which arms the RX ring + enables MAC RX via ops->start),
	 * set the peer, and start the connect state machine (scan->auth->assoc->
	 * 4-way).  The FSM then advances as the AP's frames arrive over the HAL RX
	 * path; the connect_work poll below fires the cfg80211 completion when it
	 * reaches CONNECTED (or times out).
	 *
	 * TODO: reaching CONNECTED requires the chip to actually receive
	 * the AP's auth/assoc/EAPOL frames - i.e. working silicon with the chip
	 * bring-up applied (probe) and a reachable AP.  In sim/compile terms the
	 * poll will time out; the up-call path is wired, not the on-air exchange.
	 */
	rf_2a4m1_sme_init(&dev->sme, RF_2A4M1_SME_ROLE_STA, &dev->macaddr,
			  pmk, &rf_2a4m1_sme_crypto_impl, 0);
	rf_2a4m1_sme_bind_hal(&dev->sme, &dev->hal);
	rf_2a4m1_sme_set_peer(&dev->sme, &peer);
	rf_2a4m1_sme_connect_start(&dev->sme);

	memcpy(dev->connect_bssid, conn->bssid, RF_2A4M1_ETH_ALEN);
	dev->connect_polls = 0;

	/*
	 * Remember the target's SSID + channel so the completion can publish its
	 * BSS to cfg80211 (cfg80211 matches the BSS on BSSID + SSID), and clear
	 * any beacon captured for a previous connect so the RX worker starts this
	 * one fresh. Do this before arming connect_pending, which gates capture.
	 */
	dev->connect_ssid_len = min_t(size_t, conn->ssid_len,
				      sizeof(dev->connect_ssid));
	if (conn->ssid && dev->connect_ssid_len)
		memcpy(dev->connect_ssid, conn->ssid, dev->connect_ssid_len);
	dev->connect_chan = conn->channel;
	spin_lock(&dev->bss_frame_lock);
	dev->bss_frame_len = 0;
	dev->bss_frame_rssi = -60;	/* placeholder until a target beacon is heard */
	spin_unlock(&dev->bss_frame_lock);

	dev->connect_pending = true;
	schedule_delayed_work(&dev->connect_work,
			      msecs_to_jiffies(RF_2A4M1_CONNECT_POLL_MS));

	dev_info(dev->dev, "connect: started SME to %pM (%s; real 4-way crypto; polling for completion)\n",
		 conn->bssid, have_pmk ? "WPA2 PMK derived" : "open/no-PMK");
	return 0;
}

static int rf_2a4m1_cfg_disconnect(struct wiphy *wiphy, struct net_device *ndev,
				   u16 reason_code)
{
	struct rf_2a4m1_dev *dev = rf_2a4m1_from_wiphy(wiphy);

	dev->connect_pending = false;
	cancel_delayed_work(&dev->connect_work);
	/*
	 * The encrypted-data-plane tally: tx_ccmp = frames HW-encrypted + radiated,
	 * rx_protected = Protected data frames received, rx_decrypt_ok =
	 * HW-decrypted-OK (RXINFO.DECRYPT), rx_mic_err = ICV/MIC decrypt failures.
	 * decrypt_ok > 0 proves the RX traffic was HW-decrypted, not software CCMP.
	 */
	dev_info(dev->dev,
		 "hwcrypto counters at disconnect: tx_ccmp=%d rx_protected=%d rx_decrypt_ok=%d rx_mic_err=%d\n",
		 atomic_read(&dev->tx_data_ccmp),
		 atomic_read(&dev->rx_data_protected),
		 atomic_read(&dev->rx_data_decrypt_ok),
		 atomic_read(&dev->rx_mic_err));
	/*
	 * THE decisive line: per-class TX ACK success read from MT_TX_STAT_FIFO.
	 * ENCRYPTED success>0 => the AP ACKs our HW-encrypted frames (wall is L3);
	 * ENCRYPTED success=0 while CLEARTEXT success>0 => the HW-encrypt is the
	 * bug; BOGUS success must be 0 for the measurement to mean anything.
	 */
	rf_2a4m1_usb_stat_report(dev);
	dev->hw_key_installed = false;
	rf_2a4m1_sme_tx_protected_deauth(&dev->sme, reason_code);
	cfg80211_disconnected(ndev, reason_code, NULL, 0, true, GFP_KERNEL);
	return 0;
}

static int rf_2a4m1_cfg_add_key(struct wiphy *wiphy, struct net_device *ndev,
				int link_id, u8 key_index, bool pairwise,
				const u8 *mac_addr, struct key_params *params)
{
	struct rf_2a4m1_dev *dev = rf_2a4m1_from_wiphy(wiphy);
	struct rf_2a4m1_key k;

	memset(&k, 0, sizeof(k));
	k.type    = pairwise ? 1 : 0;
	k.cipher  = params->cipher & 0xff;
	k.key_id  = key_index;
	k.key_len = min_t(int, params->key_len, (int)sizeof(k.material));
	if (params->key)
		memcpy(k.material, params->key, k.key_len);
	if (mac_addr)
		memcpy(k.peer.a, mac_addr, RF_2A4M1_ETH_ALEN);

	/* Downward face: program the key through the kernel USB HAL. */
	if (dev->hal.ops && dev->hal.ops->set_key)
		dev->hal.ops->set_key(&dev->hal, key_index, &k);
	return 0;
}

static int rf_2a4m1_cfg_del_key(struct wiphy *wiphy, struct net_device *ndev,
				int link_id, u8 key_index, bool pairwise,
				const u8 *mac_addr)
{
	struct rf_2a4m1_dev *dev = rf_2a4m1_from_wiphy(wiphy);
	struct rf_2a4m1_key k;

	memset(&k, 0, sizeof(k));	/* cipher 0 = clear the slot */
	k.key_id = key_index;
	if (dev->hal.ops && dev->hal.ops->set_key)
		dev->hal.ops->set_key(&dev->hal, key_index, &k);
	return 0;
}

static int rf_2a4m1_cfg_set_default_key(struct wiphy *wiphy,
					struct net_device *ndev, int link_id,
					u8 key_index, bool unicast,
					bool multicast)
{
	return 0;	/* single default-key slot for this silicon */
}

static int rf_2a4m1_cfg_change_iface(struct wiphy *wiphy,
				     struct net_device *ndev,
				     enum nl80211_iftype type,
				     struct vif_params *params)
{
	if (type != NL80211_IFTYPE_STATION &&
	    type != NL80211_IFTYPE_AP &&
	    type != NL80211_IFTYPE_MONITOR)
		return -EOPNOTSUPP;
	ndev->ieee80211_ptr->iftype = type;
	/* TODO: push the opmode down via hal set_rx_filter. */
	return 0;
}

static int rf_2a4m1_cfg_get_station(struct wiphy *wiphy,
				    struct net_device *ndev,
				    const u8 *mac,
				    struct station_info *sinfo)
{
	/* TODO: fill rate/RSSI from the RXWI the core last saw. */
	sinfo->filled = 0;
	return 0;
}

static const struct cfg80211_ops rf_2a4m1_cfg80211_ops = {
	.scan			= rf_2a4m1_cfg_scan,
	.connect		= rf_2a4m1_cfg_connect,
	.disconnect		= rf_2a4m1_cfg_disconnect,
	.add_key		= rf_2a4m1_cfg_add_key,
	.del_key		= rf_2a4m1_cfg_del_key,
	.set_default_key	= rf_2a4m1_cfg_set_default_key,
	.change_virtual_intf	= rf_2a4m1_cfg_change_iface,
	.get_station		= rf_2a4m1_cfg_get_station,
};

/* ================================================================== */
/* Registration.                                                      */
/* ================================================================== */
struct wiphy *rf_2a4m1_wiphy_alloc(struct device *parent)
{
	struct wiphy *wiphy;

	wiphy = wiphy_new_nm(&rf_2a4m1_cfg80211_ops,
			     sizeof(struct rf_2a4m1_dev *), "rf-2a4m1");
	if (!wiphy)
		return NULL;

	set_wiphy_dev(wiphy, parent);
	wiphy->interface_modes = BIT(NL80211_IFTYPE_STATION) |
				 BIT(NL80211_IFTYPE_AP) |
				 BIT(NL80211_IFTYPE_MONITOR);
	wiphy->bands[NL80211_BAND_2GHZ] = &rf_2a4m1_band_2ghz;
	wiphy->cipher_suites = rf_2a4m1_cipher_suites;
	wiphy->n_cipher_suites = ARRAY_SIZE(rf_2a4m1_cipher_suites);
	wiphy->max_scan_ssids = 4;
	wiphy->max_scan_ie_len = 2048;
	wiphy->signal_type = CFG80211_SIGNAL_TYPE_MBM;
	wiphy->flags |= WIPHY_FLAG_HAVE_AP_SME;

	return wiphy;
}

int rf_2a4m1_cfg80211_register(struct rf_2a4m1_dev *dev)
{
	struct net_device *ndev;
	int ret;

	rf_2a4m1_set_wiphy_dev(dev->wiphy, dev);

	spin_lock_init(&dev->bss_frame_lock);
	INIT_DELAYED_WORK(&dev->connect_work, rf_2a4m1_connect_worker);
	rf_2a4m1_usb_stat_init(dev);

	ret = wiphy_register(dev->wiphy);
	if (ret) {
		dev_err(dev->dev, "wiphy_register failed: %d\n", ret);
		return ret;
	}

	ndev = alloc_netdev(sizeof(struct rf_2a4m1_dev *), "wlan%d",
			    NET_NAME_ENUM, ether_setup);
	if (!ndev) {
		ret = -ENOMEM;
		goto err_wiphy;
	}
	rf_2a4m1_set_ndev_dev(ndev, dev);
	dev->ndev = ndev;

	dev->wdev.wiphy = dev->wiphy;
	dev->wdev.netdev = ndev;
	dev->wdev.iftype = NL80211_IFTYPE_STATION;
	ndev->ieee80211_ptr = &dev->wdev;
	ndev->netdev_ops = &rf_2a4m1_netdev_ops;
	SET_NETDEV_DEV(ndev, wiphy_dev(dev->wiphy));
	eth_hw_addr_set(ndev, dev->macaddr.a);

	ret = register_netdev(ndev);
	if (ret) {
		dev_err(dev->dev, "register_netdev failed: %d\n", ret);
		goto err_free;
	}
	return 0;

err_free:
	free_netdev(ndev);
	dev->ndev = NULL;
err_wiphy:
	wiphy_unregister(dev->wiphy);
	return ret;
}

void rf_2a4m1_cfg80211_unregister(struct rf_2a4m1_dev *dev)
{
	dev->connect_pending = false;
	cancel_delayed_work_sync(&dev->connect_work);
	rf_2a4m1_usb_stat_stop(dev);
	if (dev->ndev) {
		unregister_netdev(dev->ndev);
		free_netdev(dev->ndev);
		dev->ndev = NULL;
	}
	if (dev->wiphy)
		wiphy_unregister(dev->wiphy);
}
