// SPDX-License-Identifier: GPL-2.0-only
/*
 * RF-2A4M1 - kernel USB HAL.
 *
 * Realizes the core's HAL vtable (struct rf_2a4m1_hal_ops) over
 * usbcore: the MT7601U vendor control-transfer register plane, the FCE bulk
 * DMA firmware upload, and the bulk TX/RX data path.  The byte-exact TXWI /
 * RXWI descriptor build+parse and the USB DMA framing are the core's
 * pure functions (rf_2a4m1_mt7601u_build_txwi / _usb_tx_wrap / _usb_rx_unwrap /
 * _parse_rxwi); this file supplies only the real kernel transport under them.
 *
 * The MT7601U register offsets, vendor requests, and firmware-container layout
 * are the public MediaTek MT7601U hardware map (as documented by the mainline
 * in-kernel mt7601u driver: usb.c / mcu.c / regs.h / dma.h).
 *
 * Copyright (C) GenBasic.
 */
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/etherdevice.h>
#include <linux/netdevice.h>
#include <linux/ieee80211.h>
#include <net/cfg80211.h>

#include "rf-2a4m1.h"

/* ---- vendor requests (MT7601U bRequest values) ---- */
#define MT_VEND_DEV_MODE	1
#define MT_VEND_WRITE		2
#define MT_VEND_MULTI_READ	7
#define MT_VEND_WRITE_FCE	0x42
#define MT_VEND_DEV_MODE_RESET	1
#define MT_VEND_REQ_MAX_RETRY	10
#define MT_VEND_REQ_TOUT_MS	300

/* ---- registers used by the firmware-load path ---- */
#define MT_USB_DMA_CFG			0x0238
#define MT_USB_DMA_CFG_TX_CLR		(1u << 19)
#define MT_USB_DMA_CFG_RX_BULK_EN	(1u << 22)
#define MT_USB_DMA_CFG_TX_BULK_EN	(1u << 23)
#define MT_FCE_DMA_ADDR			0x0230
#define MT_FCE_DMA_LEN			0x0234
#define MT_PBF_CFG			0x0404
#define MT_PBF_CFG_TXQ_EN		0x0f	/* TX0..3 queue enable */
#define MT_MCU_COM_REG0			0x0730
#define MT_MCU_COM_REG1			0x0734
#define MT_FCE_PSE_CTRL			0x0800
#define MT_TX_CPU_FROM_FCE_BASE_PTR	0x09a0
#define MT_TX_CPU_FROM_FCE_MAX_COUNT	0x09a4
#define MT_TX_CPU_FROM_FCE_CPU_DESC_IDX	0x09a8
#define MT_FCE_PDMA_GLOBAL_CONF		0x09c4
#define MT_FCE_SKIP_FS			0x0a6c

/* ---- MCU firmware container ---- */
#define MT_MCU_IVB_SIZE			0x40
#define MT_MCU_DLM_OFFSET		0x80000
#define MCU_FW_URB_MAX_PAYLOAD		0x3800

/* DMA TXINFO: D_PORT = CPU_TX_PORT(2). */
#define MT_TXD_INFO_D_PORT_CPU_TX	(2u << 27)

/* EDCA qsel for a normal data frame. */
#define MT_QSEL_EDCA			2

static inline u32 rf_2a4m1_roundup4(u32 x) { return (x + 3u) & ~3u; }

/* ------------------------------------------------------------------ */
/* Vendor control transfer (mirrors the mt7601u vendor_request retry loop). */
static int rf_2a4m1_vendor_request(struct rf_2a4m1_dev *dev, u8 req, u8 dir,
				   u16 val, u16 offset, void *buf, u16 len)
{
	struct usb_device *udev = dev->udev;
	unsigned int pipe = (dir & USB_DIR_IN) ? usb_rcvctrlpipe(udev, 0)
					       : usb_sndctrlpipe(udev, 0);
	u8 reqtype = dir | USB_TYPE_VENDOR | USB_RECIP_DEVICE;
	int ret = -ENODEV;
	int i;

	for (i = 0; i < MT_VEND_REQ_MAX_RETRY; i++) {
		ret = usb_control_msg(udev, pipe, req, reqtype, val, offset,
				      buf, len, MT_VEND_REQ_TOUT_MS);
		if (ret >= 0 || ret == -ENODEV)
			return ret;
		usleep_range(5000, 6000);
	}
	dev_err(dev->dev, "vendor req %02x off %04x failed: %d\n",
		req, offset, ret);
	return ret;
}

/* Register read over MT_VEND_MULTI_READ (IN, 4 bytes). ~0 on error. */
u32 rf_2a4m1_reg_read(struct rf_2a4m1_dev *dev, u16 offset)
{
	__le32 *buf;
	u32 val = ~0u;
	int ret;

	buf = kzalloc(sizeof(*buf), GFP_KERNEL);
	if (!buf)
		return ~0u;
	ret = rf_2a4m1_vendor_request(dev, MT_VEND_MULTI_READ, USB_DIR_IN,
				      0, offset, buf, sizeof(*buf));
	if (ret == sizeof(*buf))
		val = le32_to_cpu(*buf);
	kfree(buf);
	return val;
}

/* A 32-bit register write is two 16-bit control writes (no data stage). */
static int rf_2a4m1_single_wr(struct rf_2a4m1_dev *dev, u8 req,
			      u16 offset, u32 val)
{
	int ret = rf_2a4m1_vendor_request(dev, req, USB_DIR_OUT,
					  val & 0xffff, offset, NULL, 0);
	if (ret >= 0)
		ret = rf_2a4m1_vendor_request(dev, req, USB_DIR_OUT,
					      val >> 16, offset + 2, NULL, 0);
	return ret;
}

int rf_2a4m1_reg_write(struct rf_2a4m1_dev *dev, u16 offset, u32 val)
{
	return rf_2a4m1_single_wr(dev, MT_VEND_WRITE, offset, val);
}

static int rf_2a4m1_reg_write_fce(struct rf_2a4m1_dev *dev, u16 offset, u32 val)
{
	return rf_2a4m1_single_wr(dev, MT_VEND_WRITE_FCE, offset, val);
}

static u32 rf_2a4m1_reg_rmw(struct rf_2a4m1_dev *dev, u16 offset,
			    u32 mask, u32 val)
{
	val |= rf_2a4m1_reg_read(dev, offset) & ~mask;
	rf_2a4m1_reg_write(dev, offset, val);
	return val;
}

static void rf_2a4m1_vendor_reset(struct rf_2a4m1_dev *dev)
{
	rf_2a4m1_vendor_request(dev, MT_VEND_DEV_MODE, USB_DIR_OUT,
				MT_VEND_DEV_MODE_RESET, 0, NULL, 0);
}

static int rf_2a4m1_poll(struct rf_2a4m1_dev *dev, u16 reg,
			 u32 mask, u32 val, int timeout_ms)
{
	int i;

	for (i = 0; i < timeout_ms; i++) {
		if ((rf_2a4m1_reg_read(dev, reg) & mask) == val)
			return 1;
		usleep_range(1000, 1500);
	}
	return 0;
}

/* ------------------------------------------------------------------ */
/* Bulk transfers. */
/* Synchronous bulk-OUT. SLEEPS -- process context only (the MCU command path). */
static int rf_2a4m1_bulk_out(struct rf_2a4m1_dev *dev, int ep_idx,
			     const void *data, int len, int *sent)
{
	unsigned int pipe;

	if (ep_idx >= dev->n_out)
		return -EINVAL;
	pipe = usb_sndbulkpipe(dev->udev, dev->out_eps[ep_idx] & USB_ENDPOINT_NUMBER_MASK);
	return usb_bulk_msg(dev->udev, pipe, (void *)data, len, sent, 1000);
}

/* Async TX URB completion: the transfer buffer and the URB are ours alone. */
static void rf_2a4m1_tx_complete(struct urb *urb)
{
	kfree(urb->transfer_buffer);
	usb_free_urb(urb);
}

/*
 * Asynchronous bulk-OUT -- ATOMIC-SAFE, and the only legal way to radiate a
 * frame from the RX completion path.
 *
 * The core drives TX from its RX handler (an AP's probe-response provokes the
 * auth frame, its auth response the assoc, and so on), which runs in softirq.
 * usb_bulk_msg() allocates a URB and SLEEPS until it completes, so calling it
 * from there is a "sleeping function called from invalid context" BUG that
 * cascades into a panic. Hand the frame to usbcore and return instead; @data
 * must be heap-allocated and is owned by the completion handler on success.
 */
static int rf_2a4m1_bulk_out_async(struct rf_2a4m1_dev *dev, int ep_idx,
				   void *data, int len)
{
	struct urb *urb;
	unsigned int pipe;
	int ret;

	if (ep_idx >= dev->n_out)
		return -EINVAL;
	urb = usb_alloc_urb(0, GFP_ATOMIC);
	if (!urb)
		return -ENOMEM;
	pipe = usb_sndbulkpipe(dev->udev,
			       dev->out_eps[ep_idx] & USB_ENDPOINT_NUMBER_MASK);
	usb_fill_bulk_urb(urb, dev->udev, pipe, data, len,
			  rf_2a4m1_tx_complete, dev);
	usb_anchor_urb(urb, &dev->tx_anchor);
	ret = usb_submit_urb(urb, GFP_ATOMIC);
	if (ret) {
		usb_unanchor_urb(urb);
		usb_free_urb(urb);	/* caller still owns @data */
		return ret;
	}
	return 0;
}

/* ------------------------------------------------------------------ */
/* Endpoint discovery: record the bulk IN/OUT addresses in enumeration order. */
static void rf_2a4m1_rx_work(struct work_struct *work);

int rf_2a4m1_usb_probe_setup(struct rf_2a4m1_dev *dev)
{
	struct usb_host_interface *alt = dev->intf->cur_altsetting;
	int i;

	init_usb_anchor(&dev->tx_anchor);
	skb_queue_head_init(&dev->rx_queue);
	INIT_WORK(&dev->rx_work, rf_2a4m1_rx_work);
	dev->n_in = dev->n_out = 0;
	for (i = 0; i < alt->desc.bNumEndpoints; i++) {
		struct usb_endpoint_descriptor *ep = &alt->endpoint[i].desc;

		if (!usb_endpoint_xfer_bulk(ep))
			continue;
		if (usb_endpoint_dir_in(ep)) {
			if (dev->n_in < RF_2A4M1_N_EP_IN)
				dev->in_eps[dev->n_in++] = ep->bEndpointAddress;
		} else {
			if (dev->n_out < RF_2A4M1_N_EP_OUT)
				dev->out_eps[dev->n_out++] = ep->bEndpointAddress;
		}
	}
	if (dev->n_in < 1 || dev->n_out < 1) {
		dev_err(dev->dev, "missing bulk endpoints (in=%d out=%d)\n",
			dev->n_in, dev->n_out);
		return -ENODEV;
	}
	dev_info(dev->dev, "endpoints: %d bulk-OUT, %d bulk-IN\n",
		 dev->n_out, dev->n_in);
	return 0;
}

/* ------------------------------------------------------------------ */
/* Firmware upload: FCE-DMA one <= MCU_FW_URB_MAX_PAYLOAD chunk to dst_addr. */
static int rf_2a4m1_dma_fw_chunk(struct rf_2a4m1_dev *dev, u8 *dbuf,
				 const u8 *data, u32 len, u32 dst_addr)
{
	u32 info = MT_TXD_INFO_D_PORT_CPU_TX | (len & 0xffffu);
	u32 rlen = rf_2a4m1_roundup4(len);
	int total = 4 + (int)rlen + 4;
	int sent = 0, ret;
	u32 idx;

	put_unaligned_le32(info, dbuf);
	memcpy(dbuf + 4, data, len);
	memset(dbuf + 4 + len, 0, 8);

	ret = rf_2a4m1_reg_write_fce(dev, MT_FCE_DMA_ADDR, dst_addr);
	if (ret < 0)
		return ret;
	ret = rf_2a4m1_reg_write_fce(dev, MT_FCE_DMA_LEN, rlen << 16);
	if (ret < 0)
		return ret;

	ret = rf_2a4m1_bulk_out(dev, RF_2A4M1_EP_OUT_CMD, dbuf, total, &sent);
	if (ret < 0) {
		dev_err(dev->dev, "fw dma bulk-out failed: %d\n", ret);
		return ret;
	}
	if (sent != total)
		dev_warn(dev->dev, "fw dma short xfer: %d != %d\n", sent, total);

	idx = rf_2a4m1_reg_read(dev, MT_TX_CPU_FROM_FCE_CPU_DESC_IDX) + 1;
	rf_2a4m1_reg_write(dev, MT_TX_CPU_FROM_FCE_CPU_DESC_IDX, idx);
	return 0;
}

static int rf_2a4m1_dma_fw(struct rf_2a4m1_dev *dev, u8 *dbuf,
			   const u8 *data, int len, u32 dst)
{
	while (len > 0) {
		int n = min(len, MCU_FW_URB_MAX_PAYLOAD);
		int ret = rf_2a4m1_dma_fw_chunk(dev, dbuf, data, n, dst);

		if (ret < 0)
			return ret;
		if (!rf_2a4m1_poll(dev, MT_MCU_COM_REG1, BIT(31), BIT(31), 500)) {
			dev_err(dev->dev, "fw dma COM_REG1 bit31 timeout @0x%x\n", dst);
			return -ETIMEDOUT;
		}
		data += n;
		len -= n;
		dst += n;
	}
	return 0;
}

/* ---- WLAN-core power-on / clock / PLL lock (init.c mt7601u_chip_onoff) ---- */
#define MT_CMB_CTRL			0x0020
#define MT_CMB_CTRL_XTAL_RDY		(1u << 22)
#define MT_CMB_CTRL_PLL_LD		(1u << 23)
#define MT_WLAN_FUN_CTRL		0x0080
#define MT_WLAN_FUN_CTRL_WLAN_EN	(1u << 0)
#define MT_WLAN_FUN_CTRL_WLAN_CLK_EN	(1u << 1)
#define MT_WLAN_FUN_CTRL_WLAN_RESET_RF	(1u << 2)
#define MT_WLAN_FUN_CTRL_WLAN_RESET	(1u << 3)
#define MT_WLAN_FUN_CTRL_FRC_WL_ANT_SEL	(1u << 5)
#define MT_WLAN_FUN_CTRL_GPIO_OUT_EN	0xff000000u

/*
 * Power on + reset-pulse + clock the WLAN core, then wait for the PLL/XTAL to
 * lock.  MUST run before firmware load and any MCU command: the MCU boots from
 * ILM (COM_REG0 == 1) without the WLAN clock, but a command that writes the
 * unclocked WLAN register/memory space (the first is BURST_WRITE to WCID mem)
 * stalls in the MCU and never emits CMD_DONE (the -110 cmd-resp silence).
 * Mainline keeps WLAN_CLK on for exactly this reason.
 */
static void rf_2a4m1_wlan_onoff_reset(struct rf_2a4m1_dev *dev)
{
	u32 v = rf_2a4m1_reg_read(dev, MT_WLAN_FUN_CTRL);
	int i, ok = 0;

	/* Force WLAN_EN + GPIO out-enable, then pulse RESET | RESET_RF. */
	v |= MT_WLAN_FUN_CTRL_GPIO_OUT_EN;
	v &= ~MT_WLAN_FUN_CTRL_FRC_WL_ANT_SEL;
	v |= MT_WLAN_FUN_CTRL_WLAN_EN;
	v |= MT_WLAN_FUN_CTRL_WLAN_RESET | MT_WLAN_FUN_CTRL_WLAN_RESET_RF;
	rf_2a4m1_reg_write(dev, MT_WLAN_FUN_CTRL, v);
	usleep_range(1000, 1500);
	v &= ~(MT_WLAN_FUN_CTRL_WLAN_RESET | MT_WLAN_FUN_CTRL_WLAN_RESET_RF);
	rf_2a4m1_reg_write(dev, MT_WLAN_FUN_CTRL, v);
	usleep_range(1000, 1500);

	/* Enable the WLAN clock, then poll CMB_CTRL for PLL/XTAL lock. */
	v |= MT_WLAN_FUN_CTRL_WLAN_EN | MT_WLAN_FUN_CTRL_WLAN_CLK_EN;
	rf_2a4m1_reg_write(dev, MT_WLAN_FUN_CTRL, v);
	usleep_range(1000, 1500);

	for (i = 0; i < 200; i++) {
		u32 c = rf_2a4m1_reg_read(dev, MT_CMB_CTRL);

		if ((c & MT_CMB_CTRL_XTAL_RDY) && (c & MT_CMB_CTRL_PLL_LD)) {
			ok = 1;
			break;
		}
		usleep_range(1000, 1500);
	}
	if (!ok)
		dev_warn(dev->dev, "WLAN PLL/XTAL not ready (CMB_CTRL=0x%08x)\n",
			 rf_2a4m1_reg_read(dev, MT_CMB_CTRL));
	else
		dev_info(dev->dev, "WLAN core enabled + clocked (PLL/XTAL locked)\n");
}

/*
 * Reimplements mt_load_firmware over kernel USB: the register setup, the
 * ILM/DLM FCE-DMA upload, the IVB push (DEV_MODE req 0x12 - starts the MCU),
 * and the COM_REG0==1 boot confirm.  fw is the mt76-container image
 * (32-B header + IVB + ILM[+DLM]).
 */
int rf_2a4m1_usb_load_firmware(struct rf_2a4m1_dev *dev,
			       const u8 *fw, size_t fw_len)
{
	u32 ilm_len_hdr, dlm_len, ilm_len;
	const u8 *ivb, *ilm;
	u8 *dbuf, *ivb_copy;
	int ret, i;

	if (fw_len < 32) {
		dev_err(dev->dev, "firmware too small (%zu)\n", fw_len);
		return -EINVAL;
	}
	ilm_len_hdr = get_unaligned_le32(fw + 0);
	dlm_len     = get_unaligned_le32(fw + 4);
	if (ilm_len_hdr <= MT_MCU_IVB_SIZE) {
		dev_err(dev->dev, "bad ilm_len %u\n", ilm_len_hdr);
		return -EINVAL;
	}
	if (fw_len != 32u + ilm_len_hdr + dlm_len) {
		dev_err(dev->dev, "fw size %zu != 32+ilm(%u)+dlm(%u)\n",
			fw_len, ilm_len_hdr, dlm_len);
		return -EINVAL;
	}
	ivb     = fw + 32;
	ilm     = fw + 32 + MT_MCU_IVB_SIZE;
	ilm_len = ilm_len_hdr - MT_MCU_IVB_SIZE;

	/* Power on + clock the WLAN core (PLL/XTAL lock) before any register setup
	 * or firmware upload -- without it the MCU boots from ILM but cannot execute
	 * commands touching the unclocked WLAN memory (the first BURST_WRITE stalls
	 * with no CMD_DONE).  Mirrors init.c mt7601u_chip_onoff (done as step 1). */
	rf_2a4m1_wlan_onoff_reset(dev);

	/* ---- register setup ---- */
	rf_2a4m1_reg_write(dev, MT_USB_DMA_CFG,
			   MT_USB_DMA_CFG_RX_BULK_EN | MT_USB_DMA_CFG_TX_BULK_EN);

	if (rf_2a4m1_reg_read(dev, MT_MCU_COM_REG0) == 1) {
		dev_info(dev->dev, "MCU already running (warm) - reset + reload\n");
		rf_2a4m1_vendor_reset(dev);
		usleep_range(5000, 6000);
		rf_2a4m1_reg_write(dev, MT_MCU_COM_REG0, 0);
	}

	rf_2a4m1_reg_write(dev, 0x94c, 0);
	rf_2a4m1_reg_write(dev, MT_FCE_PSE_CTRL, 0);

	rf_2a4m1_vendor_reset(dev);
	usleep_range(5000, 6000);

	rf_2a4m1_reg_write(dev, 0xa44, 0);
	rf_2a4m1_reg_write(dev, 0x230, 0x84210);
	rf_2a4m1_reg_write(dev, 0x400, 0x80c00);
	rf_2a4m1_reg_write(dev, 0x800, 1);

	rf_2a4m1_reg_rmw(dev, MT_PBF_CFG, 0, MT_PBF_CFG_TXQ_EN);
	rf_2a4m1_reg_write(dev, MT_FCE_PSE_CTRL, 1);

	rf_2a4m1_reg_write(dev, MT_USB_DMA_CFG,
			   MT_USB_DMA_CFG_RX_BULK_EN | MT_USB_DMA_CFG_TX_BULK_EN);
	{
		u32 v = rf_2a4m1_reg_rmw(dev, MT_USB_DMA_CFG, 0,
					 MT_USB_DMA_CFG_TX_CLR);
		v &= ~MT_USB_DMA_CFG_TX_CLR;
		rf_2a4m1_reg_write(dev, MT_USB_DMA_CFG, v);
	}

	rf_2a4m1_reg_write(dev, MT_TX_CPU_FROM_FCE_BASE_PTR, 0x400230);
	rf_2a4m1_reg_write(dev, MT_TX_CPU_FROM_FCE_MAX_COUNT, 1);
	rf_2a4m1_reg_write(dev, MT_FCE_PDMA_GLOBAL_CONF, 0x44);
	rf_2a4m1_reg_write(dev, MT_FCE_SKIP_FS, 3);

	/* ---- upload ---- */
	dbuf = kmalloc(MCU_FW_URB_MAX_PAYLOAD + 12, GFP_KERNEL);
	if (!dbuf)
		return -ENOMEM;

	dev_info(dev->dev, "uploading ILM %u B -> 0x%x\n", ilm_len, MT_MCU_IVB_SIZE);
	ret = rf_2a4m1_dma_fw(dev, dbuf, ilm, (int)ilm_len, MT_MCU_IVB_SIZE);
	if (ret < 0)
		goto out;

	if (dlm_len) {
		dev_info(dev->dev, "uploading DLM %u B -> 0x%x\n",
			 dlm_len, MT_MCU_DLM_OFFSET);
		ret = rf_2a4m1_dma_fw(dev, dbuf, ilm + ilm_len, (int)dlm_len,
				      MT_MCU_DLM_OFFSET);
		if (ret < 0)
			goto out;
	}

	/* IVB last, via DEV_MODE req 0x12 - this starts the MCU. */
	ivb_copy = kmemdup(ivb, MT_MCU_IVB_SIZE, GFP_KERNEL);
	if (!ivb_copy) {
		ret = -ENOMEM;
		goto out;
	}
	ret = rf_2a4m1_vendor_request(dev, MT_VEND_DEV_MODE, USB_DIR_OUT,
				      0x12, 0, ivb_copy, MT_MCU_IVB_SIZE);
	kfree(ivb_copy);
	if (ret < 0) {
		dev_err(dev->dev, "IVB DEV_MODE failed: %d\n", ret);
		goto out;
	}

	/* Boot confirm: poll COM_REG0 == 1. */
	ret = -ETIMEDOUT;
	for (i = 100; i; i--) {
		if (rf_2a4m1_reg_read(dev, MT_MCU_COM_REG0) == 1) {
			ret = 0;
			break;
		}
		usleep_range(10000, 11000);
	}
	if (ret == 0) {
		dev_info(dev->dev, "MCU running - COM_REG0 == 1\n");
		dev->state = RF_2A4M1_STATE_FW_LOADED;
	} else {
		dev_err(dev->dev, "MCU boot timeout (COM_REG0=0x%x)\n",
			rf_2a4m1_reg_read(dev, MT_MCU_COM_REG0));
	}
out:
	kfree(dbuf);
	return ret;
}

/* ================================================================== */
/* MAC/BBP/RF chip bring-up.                                          */
/*                                                                    */
/* The register init sequence run once after the on-chip MCU firmware */
/* boots, so RX/TX carry real frames: the MAC CSR init-vals, the BBP  */
/* and RF register tables, the efuse (EEPROM) read, the per-channel RF */
/* program, and the MCU-run RF calibrations.  Register offsets, the    */
/* register-value tables, and the MCU command framing are the public   */
/* MediaTek MT7601U hardware map (as documented by the mainline        */
/* in-kernel mt7601u driver: init.c / phy.c / mcu.c / eeprom.c /       */
/* initvals.h / initvals_phy.h / regs.h).                             */
/*                                                                    */
/* The BBP and RF register files are reached through their direct CSR  */
/* bridges (MT_BBP_CSR_CFG / MT_RF_CSR_CFG); the initval tables that   */
/* mainline batches over the MCU RANDOM_WRITE command are written one  */
/* register at a time through those bridges here, for the same end     */
/* register state without the batched-command path.  The MCU command   */
/* path is used only where the operation is MCU-performed (Q_SELECT,   */
/* the RF calibrations, the BW filter, the WCID-table clear); it runs  */
/* synchronously at bring-up, before the async RX ring is armed, so    */
/* the command-response endpoint is quiescent.                        */
/* ================================================================== */

/* MAC CSRs. */
#define MT_MAC_SYS_CTRL			0x1004
#define MT_MAC_SYS_CTRL_RESET_CSR	BIT(0)
#define MT_MAC_SYS_CTRL_RESET_BBP	BIT(1)
#define MT_MAC_SYS_CTRL_ENABLE_TX	BIT(2)
#define MT_MAC_SYS_CTRL_ENABLE_RX	BIT(3)
#define MT_MAC_ADDR_DW0			0x1008
#define MT_MAC_ADDR_DW1			0x100c
#define MT_WPDMA_GLO_CFG		0x0208
#define MT_WPDMA_GLO_CFG_TX_DMA_BUSY	BIT(1)
#define MT_WPDMA_GLO_CFG_RX_DMA_BUSY	BIT(3)
#define MT_USB_DMA_CFG_RX_BULK_AGG_EN	BIT(21)
#define MT_USB_DMA_CFG_UDMA_RX_WL_DROP	BIT(25)
#define MT_USB_AGGR_SIZE_LIMIT		28
#define MT_USB_AGGR_TIMEOUT		0x80
#define MT_BCN_OFFSET_BASE		0x041c
#define MT_AUX_CLK_CFG			0x120c
#define MT_US_CYC_CFG			0x02a4
#define MT_US_CYC_CNT			0x000000ffu
#define MT_TXOP_CTRL_CFG		0x1340
#define MT_BEACON_TIME_CFG		0x1114
#define MT_BEACON_TIME_CFG_MASK		0x001f0000u	/* TIMER_EN|SYNC|TBTT|BEACON_TX */
#define MT_RX_STA_CNT0			0x1700
#define MT_TX_STA_CNT0			0x170c
/*
 * MAC TX-status report FIFO (MT_TX_STAT_FIFO @ 0x1718): a 32-bit MMIO register
 * that pops one TX-status report per read.  The MAC posts a report for every TX
 * frame whose TXWI carries a nonzero pktid (pktid 0 => no report); reading the
 * register returns the oldest report and consumes it, VALID clears when empty.
 * The decisive field is SUCCESS (BIT5): the frame's unicast RA (A1) ACK'd it.
 * Layout is the public MediaTek MT7601U hardware map (the mainline in-kernel
 * mt7601u driver: regs.h MT_TX_STAT_FIFO_* + mac.c mt7601u_mac_fetch_tx_status).
 */
#define MT_TX_STAT_FIFO			0x1718
#define MT_TXS_FIFO_VALID		BIT(0)		/* slot carries a report */
#define MT_TXS_FIFO_PID_SHIFT		1		/* GENMASK(4,1): pktid cookie */
#define MT_TXS_FIFO_PID_MASK		0x0000001eu
#define MT_TXS_FIFO_SUCCESS		BIT(5)		/* frame was ACK'd */
#define MT_TXS_FIFO_AGGR		BIT(6)		/* reported aggregated */
#define MT_TXS_FIFO_ACKREQ		BIT(7)		/* an ACK was requested */
#define MT_TXS_FIFO_WCID_SHIFT		8		/* GENMASK(15,8): peer/WCID slot */
#define MT_TXS_FIFO_WCID_MASK		0x0000ff00u
#define MT_TXS_FIFO_RATE_SHIFT		16		/* GENMASK(31,16): rate word */

/*
 * TXWI len_ctl (__le16 @ offset 6): a 12-bit MPDU byte count + a 4-bit pktid in
 * bits 15:12.  Stamping a nonzero pktid is what makes the MAC post a TX-status
 * report for the frame (mainline mt7601u_tx_pktid_enc).  build_txwi already
 * stamps tp->pktid from the mt7601u len_ctl layout; the
 * cleartext class is tagged in op_tx via rf_2a4m1_txwi_set_pktid().
 */
#define MT_TXWI_OFF_LEN			6
#define MT_TXWI_LEN_PKTID_SHIFT		12
#define MT_TXWI_LEN_PKTID_MASK		0xf000u

/*
 * TX-status correlation pktids, one per frame class, so the FIFO drain can split
 * ACK success by class (4-bit field; 0 = no report).  CLEARTEXT is stamped in
 * op_tx onto the WIV=1 connect mgmt/EAPOL frames; ENCRYPTED and BOGUS are set by
 * their callers via tp->pktid.
 */
#define RF_2A4M1_PID_CLEARTEXT		1	/* WIV=1 connect mgmt / EAPOL       */
#define RF_2A4M1_PID_ENCRYPTED		2	/* WIV=0 HW-CCMP data frame to AP   */
#define RF_2A4M1_PID_BOGUS		3	/* WIV=0 frame to a bogus BSSID     */

/*
 * RXWI rxinfo status word (the first word of the RXWI). BIT(14) flags the 2
 * bytes of L2 padding the MAC inserts between a non-4-byte-aligned 802.11
 * header and the payload.
 */
#define MT_RXINFO_L2PAD			BIT(14)

#define MT_RX_FILTR_CFG			0x1400
#define MT_RX_FILTR_CFG_CRC_ERR		BIT(0)
#define MT_RX_FILTR_CFG_PHY_ERR		BIT(1)
#define MT_TX_BAND_CFG			0x132c
#define MT_TX_ALC_CFG_0			0x13b0
#define MT_TX_PWR_CFG_0			0x1314
#define MT_RF_PA_MODE_CFG0		0x121c
#define MT_RF_PA_MODE_CFG1		0x1220
#define MT_BKOFF_SLOT_CFG		0x1104
#define MT_XIFS_TIME_CFG		0x1100

/* efuse (EEPROM). */
#define MT_EFUSE_CTRL			0x0024
#define MT_EFUSE_CTRL_AOUT		0x0000003fu
#define MT_EFUSE_CTRL_MODE		0x000000c0u
#define MT_EFUSE_CTRL_AIN		0x03ff0000u
#define MT_EFUSE_CTRL_KICK		BIT(30)
#define MT_EFUSE_DATA_BASE		0x0028
#define MT_EE_MAC_ADDR			0x04
#define MT_EE_FREQ_OFFSET		0x3a
#define MT_EE_LNA_GAIN			0x44
#define MT_EE_REF_TEMP			0xd1
#define MT_EE_TX_POWER_OFFSET		0x52
#define MT_EE_TX_POWER_BYRATE_BASE	0xde
#define MT_EEPROM_SIZE			256
#define MT_DEFAULT_TX_POWER		6

/* BBP / RF direct CSR bridges. */
#define MT_RF_CSR_CFG			0x0500
#define MT_RF_CSR_CFG_KICK		BIT(31)
#define MT_RF_CSR_CFG_WR		BIT(30)
#define MT_BBP_CSR_CFG			0x101c
#define MT_BBP_CSR_CFG_BUSY		BIT(17)
#define MT_BBP_CSR_CFG_READ		BIT(16)
#define MT_BBP_CSR_CFG_RW		BIT(19)

/* WCID address CAM (per-peer TA match). */
#define MT_WCID_ADDR_BASE		0x1800
#define MT_WCID_ADDR(i)			(MT_WCID_ADDR_BASE + (i) * 8)
#define MT_SKEY_MODE_BASE_0		0xb000
#define MT_WCID_ATTR_BASE		0xa800

/*
 * HW crypto key table (mainline mt7601u regs.h / mac.c). The MT7601U CCMP engine
 * is keyed per-WCID: with a pairwise key in slot n + the peer MAC in the address
 * CAM, the chip auto-inserts the CCMP header (IV/EIV from its own PN) + encrypts
 * on TX (TXWI.wcid == n, DMA WIV=0), and auto-decrypts + strips IV/MIC on RX for
 * a Protected frame whose TA matches (RXINFO.DECRYPT). Group-addressed frames use
 * the shared key table (MT_SKEY) indexed by (BSS, KeyID). The BSSID (MYBSS)
 * filter is the gate the RX crypto engine needs before it does the key lookup.
 */
#define MT_WCID_KEY_BASE		0x8000
#define MT_WCID_KEY(i)			(MT_WCID_KEY_BASE + (i) * 32)
#define MT_WCID_IV_BASE			0xa000
#define MT_WCID_IV(i)			(MT_WCID_IV_BASE + (i) * 8)
#define MT_WCID_ATTR(i)			(MT_WCID_ATTR_BASE + (i) * 4)
#define MT_WCID_ATTR_PAIRWISE		BIT(0)
#define MT_WCID_ATTR_PKEY_MODE_SHIFT	1		/* GENMASK(3,1): cipher */
#define MT_WCID_DROP_BASE		0x106c
#define MT_WCID_DROP(i)			(MT_WCID_DROP_BASE + ((i) >> 5) * 4)
#define MT_WCID_DROP_MASK(i)		(1u << ((i) % 32))
#define MT_CIPHER_AES_CCMP		4
#define MT_MAC_BSSID_DW0		0x1010
#define MT_MAC_BSSID_DW1		0x1014
#define MT_SKEY_BASE_0			0xac00
#define MT_SKEY(bss, idx)		(MT_SKEY_BASE_0 + (4 * (bss) + (idx)) * 32)
#define MT_SKEY_MODE_0(bss)		(MT_SKEY_MODE_BASE_0 + (((bss) / 2) << 2))
#define MT_SKEY_MODE_MASK		0xfu
#define MT_SKEY_MODE_SHIFT(bss, idx)	(4 * ((idx) + 4 * ((bss) & 1)))
#define RF_2A4M1_HW_WCID		1	/* STA pairwise/data WCID slot */

/* RXWI rxinfo (first le32) HW-decrypt status bits (mainline mt7601u mac.h). */
#define MT_RXINFO_ICVERR		BIT(9)
#define MT_RXINFO_MICERR		BIT(10)
#define MT_RXINFO_DECRYPT		BIT(16)

/* MCU inband command framing. */
#define MT_MCU_MEMMAP_WLAN		0x00410000u
#define MT_MCU_INBAND_MAX_LEN		192
#define MT_TXD_CMD_INFO_SEQ_SHIFT	16
#define MT_TXD_CMD_INFO_TYPE_SHIFT	20
#define MT_TXD_INFO_TYPE_SHIFT		30
#define MT_INFO_DMA_COMMAND		1
#define MT_RXD_CMD_INFO_CMD_SEQ_SHIFT	16
#define MT_RXD_CMD_INFO_EVT_TYPE_SHIFT	20
#define MT_EVT_CMD_DONE			0
#define MT_CMD_FUN_SET_OP		1
#define MT_CMD_BURST_WRITE		8
#define MT_CMD_CALIBRATION_OP		31

/* MCU calibration ids. */
enum { MCU_CAL_R = 1, MCU_CAL_DCOC, MCU_CAL_LC, MCU_CAL_LOFT, MCU_CAL_TXIQ,
       MCU_CAL_BW, MCU_CAL_DPD, MCU_CAL_RXIQ, MCU_CAL_TXDCOC };

/* Register-pair table element types. */
struct rf_2a4m1_mac_rp { u16 reg; u32 val; };
struct rf_2a4m1_bbp_rp { u8 reg; u8 val; };
struct rf_2a4m1_rf_rp  { u8 bank, reg, val; };

/* --- MAC CSR init-vals (reg = 16-bit CSR offset). --- */
static const struct rf_2a4m1_mac_rp rf_2a4m1_mac_common[] = {
	{ 0x1408, 0x0000013f }, { 0x140c, 0x00008003 }, { 0x1004, 0x00000000 },
	{ 0x1400, 0x00017f97 }, { 0x1104, 0x00000209 }, { 0x1330, 0x00000000 },
	{ 0x1334, 0x00080606 }, { 0x1350, 0x00001020 }, { 0x1348, 0x000a2090 },
	{ 0x1018, 0x00003fff }, { 0x0408, 0x1fbf1f1f }, { 0x040c, 0x0000009f },
	{ 0x134c, 0x47d01f0f }, { 0x1404, 0x00000013 }, { 0x1364, 0x05740003 },
	{ 0x1368, 0x05740003 }, { 0x1370, 0x03f44084 }, { 0x1374, 0x01744004 },
	{ 0x1378, 0x03f44084 }, { 0x136c, 0x01744004 }, { 0x1340, 0x0000583f },
	{ 0x1344, 0x01092b20 }, { 0x1380, 0x002400ca }, { 0x1608, 0x00000002 },
	{ 0x1100, 0x33a41010 }, { 0x1204, 0x00000000 }, { 0x150c, 0x00000001 },
};
static const struct rf_2a4m1_mac_rp rf_2a4m1_mac_chip[] = {
	{ 0x0250, 0x00006050 }, { 0x041c, 0x18100800 }, { 0x0420, 0x38302820 },
	{ 0x0400, 0x00080c00 }, { 0x0404, 0x7f723c1f }, { 0x0800, 0x00000001 },
	{ 0x0a38, 0x00000000 }, { 0x13a0, 0x003b0005 }, { 0x13a8, 0x00006900 },
	{ 0x13c0, 0x00000400 }, { 0x13c8, 0x00060006 }, { 0x1330, 0x00000402 },
	{ 0x1334, 0x00000000 }, { 0x1338, 0x00000000 }, { 0x0260, 0x00000000 },
	{ 0x0808, 0x0000030f }, { 0x0804, 0x00256f0f },
};
/* --- BBP init-vals (internal BBP register file). --- */
static const struct rf_2a4m1_bbp_rp rf_2a4m1_bbp_common[] = {
	{65,0x2c},{66,0x38},{68,0x0b},{69,0x12},{70,0x0a},{73,0x10},{81,0x37},{82,0x62},
	{83,0x6a},{84,0x99},{86,0x00},{91,0x04},{92,0x00},{103,0x00},{105,0x05},{106,0x35},
};
static const struct rf_2a4m1_bbp_rp rf_2a4m1_bbp_chip[] = {
	{1,0x04},{4,0x40},{20,0x06},{31,0x08},{178,0xff},
	{66,0x14},{68,0x8b},{69,0x12},{70,0x09},{73,0x11},{75,0x60},{76,0x44},{84,0x9a},
	{86,0x38},{91,0x07},{92,0x02},
	{99,0x50},{101,0x00},{103,0xc0},{104,0x92},{105,0x3c},{106,0x03},{128,0x12},
	{142,0x04},{143,0x37},{142,0x03},{143,0x99},
	{160,0xeb},{161,0xc4},{162,0x77},{163,0xf9},{164,0x88},{165,0x80},{166,0xff},{167,0xe4},
	{195,0x00},{196,0x00},{195,0x01},{196,0x04},{195,0x02},{196,0x20},{195,0x03},{196,0x0a},
	{195,0x06},{196,0x16},{195,0x07},{196,0x05},{195,0x08},{196,0x37},{195,0x0a},{196,0x15},
	{195,0x0b},{196,0x17},{195,0x0c},{196,0x06},{195,0x0d},{196,0x09},{195,0x0e},{196,0x05},
	{195,0x0f},{196,0x09},{195,0x10},{196,0x20},{195,0x20},{196,0x17},{195,0x21},{196,0x06},
	{195,0x22},{196,0x09},{195,0x23},{196,0x17},{195,0x24},{196,0x06},{195,0x25},{196,0x09},
	{195,0x26},{196,0x17},{195,0x27},{196,0x06},{195,0x28},{196,0x09},{195,0x29},{196,0x05},
	{195,0x2a},{196,0x09},{195,0x80},{196,0x8b},{195,0x81},{196,0x12},{195,0x82},{196,0x09},
	{195,0x83},{196,0x17},{195,0x84},{196,0x11},{195,0x85},{196,0x00},{195,0x86},{196,0x00},
	{195,0x87},{196,0x18},{195,0x88},{196,0x60},{195,0x89},{196,0x44},{195,0x8a},{196,0x8b},
	{195,0x8b},{196,0x8b},{195,0x8c},{196,0x8b},{195,0x8d},{196,0x8b},{195,0x8e},{196,0x09},
	{195,0x8f},{196,0x09},{195,0x90},{196,0x09},{195,0x91},{196,0x09},{195,0x92},{196,0x11},
	{195,0x93},{196,0x11},{195,0x94},{196,0x11},{195,0x95},{196,0x11},
	{47,0x80},{60,0x80},{150,0xd2},{151,0x32},{152,0x23},{153,0x41},{154,0x00},{155,0x4f},
	{253,0x7e},{195,0x30},{196,0x32},{195,0x31},{196,0x23},{195,0x32},{196,0x45},{195,0x35},
	{196,0x4a},{195,0x36},{196,0x5a},{195,0x37},{196,0x5a},
};
/* --- RF init-vals (bank/reg/val). --- */
static const struct rf_2a4m1_rf_rp rf_2a4m1_rf_central[] = {
	{0,0,0x02},{0,1,0x01},{0,2,0x11},{0,3,0xff},{0,4,0x0a},{0,5,0x20},
	{0,6,0x00},{0,7,0x00},{0,8,0x00},{0,9,0x00},{0,10,0x00},{0,11,0x21},
	{0,13,0x00},{0,14,0x7c},{0,15,0x22},{0,16,0x80},{0,17,0x99},{0,18,0x99},
	{0,19,0x09},{0,20,0x50},{0,21,0xb0},{0,22,0x00},{0,23,0xc5},{0,24,0xfc},
	{0,25,0x40},{0,26,0x4d},{0,27,0x02},{0,28,0x72},{0,29,0x01},{0,30,0x00},
	{0,31,0x00},{0,32,0x00},{0,33,0x00},{0,34,0x23},{0,35,0x01},{0,36,0x00},
	{0,37,0x00},{0,38,0x00},{0,39,0x20},{0,40,0x00},{0,41,0xd0},{0,42,0x1b},
	{0,43,0x02},{0,44,0x00},
};
static const struct rf_2a4m1_rf_rp rf_2a4m1_rf_channel[] = {
	{4,0,0x01},{4,1,0x00},{4,2,0x00},{4,3,0x00},{4,4,0x00},{4,5,0x08},
	{4,6,0x00},{4,7,0x5b},{4,8,0x52},{4,9,0xb6},{4,10,0x57},{4,11,0x33},
	{4,12,0x22},{4,13,0x3d},{4,14,0x3e},{4,15,0x13},{4,16,0x22},{4,17,0x23},
	{4,18,0x02},{4,19,0xa4},{4,20,0x01},{4,21,0x12},{4,22,0x80},{4,23,0xb3},
	{4,24,0x00},{4,25,0x00},{4,26,0x00},{4,27,0x00},{4,28,0x18},{4,29,0xee},
	{4,30,0x6b},{4,31,0x31},{4,32,0x5d},{4,33,0x00},{4,34,0x96},{4,35,0x55},
	{4,36,0x08},{4,37,0xbb},{4,38,0xb3},{4,39,0xb3},{4,40,0x03},{4,41,0x00},
	{4,42,0x00},{4,43,0xc5},{4,44,0xc5},{4,45,0xc5},{4,46,0x07},{4,47,0xa8},
	{4,48,0xef},{4,49,0x1a},{4,54,0x07},{4,55,0xa7},{4,56,0xcc},{4,57,0x14},
	{4,58,0x07},{4,59,0xa8},{4,60,0xd7},{4,61,0x10},{4,62,0x1c},{4,63,0x00},
};
static const struct rf_2a4m1_rf_rp rf_2a4m1_rf_vga[] = {
	{5,0,0x47},{5,1,0x00},{5,2,0x00},{5,3,0x08},{5,4,0x04},{5,5,0x20},
	{5,6,0x3a},{5,7,0x3a},{5,8,0x00},{5,9,0x00},{5,10,0x10},{5,11,0x10},
	{5,12,0x10},{5,13,0x10},{5,14,0x10},{5,15,0x20},{5,16,0x22},{5,17,0x7c},
	{5,18,0x00},{5,19,0x00},{5,20,0x00},{5,21,0xf1},{5,22,0x11},{5,23,0x02},
	{5,24,0x41},{5,25,0x20},{5,26,0x00},{5,27,0xd7},{5,28,0xa2},{5,29,0x20},
	{5,30,0x49},{5,31,0x20},{5,32,0x04},{5,33,0xf1},{5,34,0xa1},{5,35,0x01},
	{5,41,0x00},{5,42,0x00},{5,43,0x00},{5,44,0x00},{5,45,0x00},{5,46,0x00},
	{5,47,0x00},{5,48,0x00},{5,49,0x00},{5,50,0x00},{5,51,0x00},{5,52,0x00},
	{5,53,0x00},{5,54,0x00},{5,55,0x00},{5,56,0x00},{5,57,0x00},{5,58,0x31},
	{5,59,0x31},{5,60,0x0a},{5,61,0x02},{5,62,0x00},{5,63,0x00},
};

/* ---- bulk-IN (used only for MCU command-response at bring-up) ---- */
static int rf_2a4m1_bulk_in(struct rf_2a4m1_dev *dev, int ep_idx,
			    void *buf, int len, int *got, int timeout_ms)
{
	unsigned int pipe;

	if (ep_idx >= dev->n_in)
		return -EINVAL;
	pipe = usb_rcvbulkpipe(dev->udev,
			       dev->in_eps[ep_idx] & USB_ENDPOINT_NUMBER_MASK);
	return usb_bulk_msg(dev->udev, pipe, buf, len, got, timeout_ms);
}

/* ---- small transforms ---- */
static int rf_2a4m1_s6_to_int(u32 reg)
{ int s6 = reg & 0x3f; if (s6 & 0x20) s6 -= 0x40; return s6; }
static u32 rf_2a4m1_int_to_s6(int val)
{ if (val < -0x20) return 0x20; if (val > 0x1f) return 0x1f; return (u32)val & 0x3f; }
static s8 rf_2a4m1_ee_field(u8 v) { return v == 0xff ? 0 : (s8)v; }

/* ---- BBP / RF direct CSR access (mirrors mt7601u_{bbp,rf}_{rr,wr}) ---- */
static int rf_2a4m1_bbp_wr(struct rf_2a4m1_dev *dev, u8 reg, u8 val)
{
	if (!rf_2a4m1_poll(dev, MT_BBP_CSR_CFG, MT_BBP_CSR_CFG_BUSY, 0, 50))
		return -ETIMEDOUT;
	return rf_2a4m1_reg_write(dev, MT_BBP_CSR_CFG,
		(u32)val | ((u32)reg << 8) | MT_BBP_CSR_CFG_RW | MT_BBP_CSR_CFG_BUSY);
}

static int rf_2a4m1_bbp_rr(struct rf_2a4m1_dev *dev, u8 reg)
{
	u32 r;

	if (!rf_2a4m1_poll(dev, MT_BBP_CSR_CFG, MT_BBP_CSR_CFG_BUSY, 0, 50))
		return -ETIMEDOUT;
	rf_2a4m1_reg_write(dev, MT_BBP_CSR_CFG, ((u32)reg << 8) |
		MT_BBP_CSR_CFG_RW | MT_BBP_CSR_CFG_BUSY | MT_BBP_CSR_CFG_READ);
	if (!rf_2a4m1_poll(dev, MT_BBP_CSR_CFG, MT_BBP_CSR_CFG_BUSY, 0, 50))
		return -ETIMEDOUT;
	r = rf_2a4m1_reg_read(dev, MT_BBP_CSR_CFG);
	if (((r >> 8) & 0xff) == reg)
		return (int)(r & 0xff);
	return -EIO;
}

static int rf_2a4m1_bbp_rmw(struct rf_2a4m1_dev *dev, u8 reg, u8 mask, u8 val)
{
	int r = rf_2a4m1_bbp_rr(dev, reg);

	if (r < 0)
		return r;
	return rf_2a4m1_bbp_wr(dev, reg, (u8)(val | ((u8)r & ~mask)));
}

static int rf_2a4m1_rf_wr(struct rf_2a4m1_dev *dev, u8 bank, u8 reg, u8 val)
{
	if (!rf_2a4m1_poll(dev, MT_RF_CSR_CFG, MT_RF_CSR_CFG_KICK, 0, 5))
		return -ETIMEDOUT;
	return rf_2a4m1_reg_write(dev, MT_RF_CSR_CFG, (u32)val |
		((u32)(reg & 0x3f) << 8) | ((u32)(bank & 0xf) << 14) |
		MT_RF_CSR_CFG_WR | MT_RF_CSR_CFG_KICK);
}

static int rf_2a4m1_rf_rr(struct rf_2a4m1_dev *dev, u8 bank, u8 reg)
{
	u32 r;

	if (!rf_2a4m1_poll(dev, MT_RF_CSR_CFG, MT_RF_CSR_CFG_KICK, 0, 5))
		return -ETIMEDOUT;
	rf_2a4m1_reg_write(dev, MT_RF_CSR_CFG, ((u32)(reg & 0x3f) << 8) |
		((u32)(bank & 0xf) << 14) | MT_RF_CSR_CFG_KICK);
	if (!rf_2a4m1_poll(dev, MT_RF_CSR_CFG, MT_RF_CSR_CFG_KICK, 0, 5))
		return -ETIMEDOUT;
	r = rf_2a4m1_reg_read(dev, MT_RF_CSR_CFG);
	if (((r >> 8) & 0x3f) == (reg & 0x3f) && ((r >> 14) & 0xf) == (bank & 0xf))
		return (int)(r & 0xff);
	return -EIO;
}

/* CMD_DONE-wait hardening for the MT7601U cmd-response path.
 * The chip's cmd-resp path shares the FCE with packet-RX: if the packet-RX FIFO
 * fills (a nearby AP beaconing once MAC-RX briefly comes up mid-calibration, or a
 * stray aggregated read), the PSE back-pressures and the shared cmd-resp path
 * stalls, wedging the MCU command engine.  A synchronous reader also can't catch a
 * response that lands in the gap between commands (a slow calibration finishing
 * after we gave up), which desyncs the seq stream.  Close both, all idempotent for
 * the commands we issue (register writes re-apply; calibrations re-run):
 *   1. before each attempt, drain stale cmd-resp + empty packet-RX (resync);
 *   2. poll several times per attempt so a slow response isn't missed;
 *   3. on no CMD_DONE, re-issue the whole command with a fresh seq, bounded. */
#define RF_2A4M1_MCU_RESP_TOUT_MS	300	/* per cmd-resp poll */
#define RF_2A4M1_MCU_RESP_TRIES		10	/* polls per attempt */
#define RF_2A4M1_MCU_CMD_ATTEMPTS	3	/* whole-command re-issues on timeout */
#define RF_2A4M1_MCU_DRAIN_TOUT_MS	5	/* short poll used to flush a FIFO */
#define RF_2A4M1_MCU_DRAIN_MAX		8	/* cap so a chatty FIFO can't spin forever */

/* True once the async RX ring is armed (interface up): it then continuously drains
 * packet-RX, so the synchronous pkt-RX drain below is both unnecessary and unsafe
 * (it would split the byte stream with the async URBs).  During probe-time chip_init
 * -- where every wait_resp MCU command runs -- the ring is not yet armed. */
static inline bool rf_2a4m1_rx_ring_armed(struct rf_2a4m1_dev *dev)
{
	return dev->rx[0].urb != NULL;
}

/* Drain a bulk-IN FIFO (cmd-resp resync, or packet-RX to relieve FCE back-pressure).
 * Bounded + short-timeout: an empty FIFO returns -ETIMEDOUT (~5 ms) and we stop.
 * buf must be DMA-mappable (kmalloc'd) and sized for one aggregated RX read. */
static int rf_2a4m1_mcu_drain(struct rf_2a4m1_dev *dev, int ep_idx, u8 *buf, int buf_sz)
{
	int drained = 0, i;

	for (i = 0; i < RF_2A4M1_MCU_DRAIN_MAX; i++) {
		int got = 0, r;

		r = rf_2a4m1_bulk_in(dev, ep_idx, buf, buf_sz, &got,
				     RF_2A4M1_MCU_DRAIN_TOUT_MS);
		if (r < 0 || got == 0)	/* timeout/error/empty => FIFO drained */
			break;
		drained++;
	}
	return drained;
}

/* ---- MCU inband command (CMD-out ep, optional CMD-resp poll) ---- */
static int rf_2a4m1_mcu_cmd(struct rf_2a4m1_dev *dev, u8 cmd,
			    const u8 *payload, int len, bool wait_resp)
{
	u32 rlen = rf_2a4m1_roundup4(len);
	int total = 4 + (int)rlen + 4;
	int sent = 0, ret, attempt;
	int resp_seen = 0, last_rc = -ETIMEDOUT;	/* -110 diagnosis */
	u32 info, last_rxfce = 0;
	u8 *buf, *drain, seq;

	if (len < 0 || (len & 3) || len > MT_MCU_INBAND_MAX_LEN)
		return -EINVAL;

	buf = kzalloc(total, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	/* Fire-and-forget (seq 0): the firmware queues no response. */
	if (!wait_resp) {
		info = (rlen & 0xffffu)
		     | ((u32)cmd << MT_TXD_CMD_INFO_TYPE_SHIFT)
		     | MT_TXD_INFO_D_PORT_CPU_TX
		     | ((u32)MT_INFO_DMA_COMMAND << MT_TXD_INFO_TYPE_SHIFT);
		put_unaligned_le32(info, buf);
		if (len)
			memcpy(buf + 4, payload, len);
		ret = rf_2a4m1_bulk_out(dev, RF_2A4M1_EP_OUT_CMD, buf, total, &sent);
		kfree(buf);
		return ret < 0 ? ret : 0;
	}

	/* wait_resp: one DMA-mappable buffer serves the FIFO drains and the cmd-resp
	 * read (an on-stack buffer trips usb_hcd_map_urb_for_dma's "on stack" WARN). */
	drain = kmalloc(RF_2A4M1_RX_BUF_SZ, GFP_KERNEL);
	if (!drain) {
		kfree(buf);
		return -ENOMEM;
	}

	ret = -ETIMEDOUT;
	for (attempt = 0; attempt < RF_2A4M1_MCU_CMD_ATTEMPTS; attempt++) {
		int tries;

		/* Resync: flush stale cmd-resp, then (while the async ring is off)
		 * empty packet-RX so it can't back-pressure the shared FCE cmd path. */
		rf_2a4m1_mcu_drain(dev, RF_2A4M1_EP_IN_CMD, drain, RF_2A4M1_RX_BUF_SZ);
		if (!rf_2a4m1_rx_ring_armed(dev))
			rf_2a4m1_mcu_drain(dev, RF_2A4M1_EP_IN_RX, drain,
					   RF_2A4M1_RX_BUF_SZ);

		seq = (dev->mcu_seq = (dev->mcu_seq % 15) + 1);
		info = (rlen & 0xffffu)
		     | ((u32)seq << MT_TXD_CMD_INFO_SEQ_SHIFT)
		     | ((u32)cmd << MT_TXD_CMD_INFO_TYPE_SHIFT)
		     | MT_TXD_INFO_D_PORT_CPU_TX
		     | ((u32)MT_INFO_DMA_COMMAND << MT_TXD_INFO_TYPE_SHIFT);
		put_unaligned_le32(info, buf);
		if (len)
			memcpy(buf + 4, payload, len);

		ret = rf_2a4m1_bulk_out(dev, RF_2A4M1_EP_OUT_CMD, buf, total, &sent);
		if (ret < 0) {
			if (ret == -ETIMEDOUT)
				continue;	/* transient bulk-out; re-issue */
			goto out;		/* real USB error */
		}

		/* Poll the cmd-resp endpoint for the matching seq + CMD_DONE. */
		for (tries = RF_2A4M1_MCU_RESP_TRIES; tries; tries--) {
			int got = 0, r;

			r = rf_2a4m1_bulk_in(dev, RF_2A4M1_EP_IN_CMD, drain,
					     RF_2A4M1_RX_BUF_SZ, &got,
					     RF_2A4M1_MCU_RESP_TOUT_MS);
			last_rc = r;
			if (r < 0) {
				if (r == -ETIMEDOUT) {
					/* Likely pkt-RX back-pressure on the shared
					 * FCE -- drain it (ring off) and keep polling. */
					if (!rf_2a4m1_rx_ring_armed(dev))
						rf_2a4m1_mcu_drain(dev,
							RF_2A4M1_EP_IN_RX, drain,
							RF_2A4M1_RX_BUF_SZ);
					continue;
				}
				ret = r;	/* real USB error -- surface it */
				goto out;
			}
			if (got >= 4) {
				u32 rxfce = get_unaligned_le32(drain);
				u8 rseq = (rxfce >> MT_RXD_CMD_INFO_CMD_SEQ_SHIFT) & 0xf;
				u8 evt  = (rxfce >> MT_RXD_CMD_INFO_EVT_TYPE_SHIFT) & 0xf;

				resp_seen++;
				last_rxfce = rxfce;
				if (rseq == seq && evt == MT_EVT_CMD_DONE) {
					ret = 0;
					goto out;
				}
			}
			/* else stale/mismatched -- keep draining within this attempt */
		}
		ret = -ETIMEDOUT;	/* no CMD_DONE this attempt -> re-issue */
	}
out:
	if (ret)
		dev_err(dev->dev,
			"mcu_cmd cmd=%u seq=%u FAILED: resp_seen=%d last_rxfce=0x%08x last_rc=%d\n",
			cmd, seq, resp_seen, last_rxfce, last_rc);
	kfree(drain);
	kfree(buf);
	return ret;
}

static int rf_2a4m1_mcu_calibrate(struct rf_2a4m1_dev *dev, u32 cal, u32 val)
{
	u8 pl[8];

	put_unaligned_le32(cal, pl);
	put_unaligned_le32(val, pl + 4);
	return rf_2a4m1_mcu_cmd(dev, MT_CMD_CALIBRATION_OP, pl, 8, true);
}

static int rf_2a4m1_mcu_function_select(struct rf_2a4m1_dev *dev, u32 func, u32 val)
{
	u8 pl[8];

	put_unaligned_le32(func, pl);
	put_unaligned_le32(val, pl + 4);
	return rf_2a4m1_mcu_cmd(dev, MT_CMD_FUN_SET_OP, pl, 8, func == 5);
}

static int rf_2a4m1_mcu_burst_write(struct rf_2a4m1_dev *dev, u32 offset,
				    const u32 *vals, int n)
{
	const int max_per = MT_MCU_INBAND_MAX_LEN / 4 - 1;	/* 47 words */
	u8 pl[MT_MCU_INBAND_MAX_LEN];
	int off = 0;

	while (off < n) {
		int cnt = min(n - off, max_per), i, ret;

		put_unaligned_le32(MT_MCU_MEMMAP_WLAN + offset + (u32)off * 4, pl);
		for (i = 0; i < cnt; i++)
			put_unaligned_le32(vals[off + i], pl + 4 + i * 4);
		ret = rf_2a4m1_mcu_cmd(dev, MT_CMD_BURST_WRITE, pl,
				       (cnt + 1) * 4, (off + cnt) == n);
		if (ret < 0)
			return ret;
		off += cnt;
	}
	return 0;
}

/* ---- table writers ---- */
static int rf_2a4m1_wr_mac_table(struct rf_2a4m1_dev *dev,
				 const struct rf_2a4m1_mac_rp *t, int n)
{
	int i, r;

	for (i = 0; i < n; i++)
		if ((r = rf_2a4m1_reg_write(dev, t[i].reg, t[i].val)) < 0)
			return r;
	return 0;
}
static int rf_2a4m1_wr_bbp_table(struct rf_2a4m1_dev *dev,
				 const struct rf_2a4m1_bbp_rp *t, int n)
{
	int i, r;

	for (i = 0; i < n; i++)
		if ((r = rf_2a4m1_bbp_wr(dev, t[i].reg, t[i].val)) < 0)
			return r;
	return 0;
}
static int rf_2a4m1_wr_rf_table(struct rf_2a4m1_dev *dev,
				const struct rf_2a4m1_rf_rp *t, int n)
{
	int i, r;

	for (i = 0; i < n; i++)
		if ((r = rf_2a4m1_rf_wr(dev, t[i].bank, t[i].reg, t[i].val)) < 0)
			return r;
	return 0;
}

/* ---- EEPROM (efuse) read ---- */
static int rf_2a4m1_efuse_read16(struct rf_2a4m1_dev *dev, u16 addr, u8 out[16])
{
	u32 val = rf_2a4m1_reg_read(dev, MT_EFUSE_CTRL);
	int i;

	val &= ~(MT_EFUSE_CTRL_AIN | MT_EFUSE_CTRL_MODE);
	val |= ((u32)(addr & ~0xf) << 16) & MT_EFUSE_CTRL_AIN;	/* MODE=READ(0) */
	val |= MT_EFUSE_CTRL_KICK;
	rf_2a4m1_reg_write(dev, MT_EFUSE_CTRL, val);
	if (!rf_2a4m1_poll(dev, MT_EFUSE_CTRL, MT_EFUSE_CTRL_KICK, 0, 100))
		return -ETIMEDOUT;
	val = rf_2a4m1_reg_read(dev, MT_EFUSE_CTRL);
	if ((val & MT_EFUSE_CTRL_AOUT) == MT_EFUSE_CTRL_AOUT) {
		memset(out, 0xff, 16);		/* hole in the usage map - ok */
		return 0;
	}
	for (i = 0; i < 4; i++)
		put_unaligned_le32(rf_2a4m1_reg_read(dev, MT_EFUSE_DATA_BASE + (i << 2)),
				   out + 4 * i);
	return 0;
}

static int rf_2a4m1_eeprom_read(struct rf_2a4m1_dev *dev)
{
	struct rf_2a4m1_eeprom *ee = &dev->ee;
	u8 e[MT_EEPROM_SIZE];
	u32 alc0, v0, v1;
	u8 max_pwr;
	int i;

	for (i = 0; i + 16 <= MT_EEPROM_SIZE; i += 16)
		if (rf_2a4m1_efuse_read16(dev, (u16)i, e + i) < 0)
			return -EIO;

	memcpy(ee->mac, e + MT_EE_MAC_ADDR, 6);
	ee->rf_freq_off = (u8)rf_2a4m1_ee_field(e[MT_EE_FREQ_OFFSET]);
	ee->ref_temp = (s8)e[MT_EE_REF_TEMP];
	ee->lna_gain = (s8)e[MT_EE_LNA_GAIN];

	alc0 = rf_2a4m1_reg_read(dev, MT_TX_ALC_CFG_0);
	max_pwr = (u8)((alc0 >> 16) & 0x3f);
	for (i = 0; i < 14; i++) {
		s8 p = rf_2a4m1_ee_field(e[MT_EE_TX_POWER_OFFSET + i]);

		if (p > (s8)max_pwr || p < 0)
			p = MT_DEFAULT_TX_POWER;
		ee->chan_pwr[i] = (u8)p;
	}
	v0 = get_unaligned_le32(e + MT_EE_TX_POWER_BYRATE_BASE);
	v1 = get_unaligned_le32(e + MT_EE_TX_POWER_BYRATE_BASE + 4);
	ee->cck_bw20[0]  = (s8)rf_2a4m1_s6_to_int(v0 & 0xff);
	ee->cck_bw20[1]  = (s8)rf_2a4m1_s6_to_int((v0 >> 8) & 0xff);
	ee->ofdm_bw20[0] = (s8)rf_2a4m1_s6_to_int((v0 >> 16) & 0xff);
	ee->ofdm_bw20[1] = (s8)rf_2a4m1_s6_to_int((v0 >> 24) & 0xff);
	ee->ofdm_bw20[2] = (s8)rf_2a4m1_s6_to_int(v1 & 0xff);
	ee->ofdm_bw20[3] = (s8)rf_2a4m1_s6_to_int((v1 >> 8) & 0xff);
	ee->valid = true;

	/* Adopt the chip MAC unless the efuse is blank/broadcast. */
	if (is_valid_ether_addr(ee->mac))
		memcpy(dev->macaddr.a, ee->mac, 6);
	return 0;
}

/* ---- PHY calibrations (MCU-performed) + channel program ---- */
static void rf_2a4m1_vco_cal(struct rf_2a4m1_dev *dev)
{
	int r;

	rf_2a4m1_rf_wr(dev, 0, 4, 0x0a);
	rf_2a4m1_rf_wr(dev, 0, 5, 0x20);
	r = rf_2a4m1_rf_rr(dev, 0, 4);
	if (r >= 0)
		rf_2a4m1_rf_wr(dev, 0, 4, (u8)(r | 0x80));
	usleep_range(2000, 3000);
}

static void rf_2a4m1_rxdc_cal(struct rf_2a4m1_dev *dev)
{
	u32 mac_ctrl = rf_2a4m1_reg_read(dev, MT_MAC_SYS_CTRL);
	int i;

	rf_2a4m1_reg_write(dev, MT_MAC_SYS_CTRL, MT_MAC_SYS_CTRL_ENABLE_RX);
	rf_2a4m1_bbp_wr(dev, 158, 0x8d); rf_2a4m1_bbp_wr(dev, 159, 0xfc);
	rf_2a4m1_bbp_wr(dev, 158, 0x8c); rf_2a4m1_bbp_wr(dev, 159, 0x4c);
	for (i = 20; i; i--) {
		usleep_range(1000, 1500);
		rf_2a4m1_bbp_wr(dev, 158, 0x8c);
		if (rf_2a4m1_bbp_rr(dev, 159) == 0x0c)
			break;
	}
	rf_2a4m1_reg_write(dev, MT_MAC_SYS_CTRL, 0);
	rf_2a4m1_bbp_wr(dev, 158, 0x8d); rf_2a4m1_bbp_wr(dev, 159, 0xe0);
	rf_2a4m1_reg_write(dev, MT_MAC_SYS_CTRL, mac_ctrl);
}

/* BW20 filter calibration (cal=true during init_cal, false at channel set). */
static int rf_2a4m1_set_bw_filter(struct rf_2a4m1_dev *dev, bool cal)
{
	u32 filter = cal ? 0 : 0x10000;	/* BW20: no 0x100 bit */
	int ret = rf_2a4m1_mcu_calibrate(dev, MCU_CAL_BW, filter | 1);	/* TX */

	if (ret < 0)
		return ret;
	return rf_2a4m1_mcu_calibrate(dev, MCU_CAL_BW, filter);		/* RX */
}

static int rf_2a4m1_init_cal(struct rf_2a4m1_dev *dev)
{
	u32 mac_ctrl = rf_2a4m1_reg_read(dev, MT_MAC_SYS_CTRL);
	int ret, r;

	if ((ret = rf_2a4m1_mcu_calibrate(dev, MCU_CAL_R, 0)) < 0)
		return ret;
	r = rf_2a4m1_rf_rr(dev, 0, 4);
	if (r >= 0)
		rf_2a4m1_rf_wr(dev, 0, 4, (u8)(r | 0x80));
	usleep_range(2000, 3000);
	if ((ret = rf_2a4m1_mcu_calibrate(dev, MCU_CAL_TXDCOC, 0)) < 0)
		return ret;
	rf_2a4m1_rxdc_cal(dev);
	if ((ret = rf_2a4m1_set_bw_filter(dev, true)) < 0)
		return ret;
	if ((ret = rf_2a4m1_mcu_calibrate(dev, MCU_CAL_LOFT, 0)) < 0)
		return ret;
	if ((ret = rf_2a4m1_mcu_calibrate(dev, MCU_CAL_TXIQ, 0)) < 0)
		return ret;
	if ((ret = rf_2a4m1_mcu_calibrate(dev, MCU_CAL_RXIQ, 0)) < 0)
		return ret;
	/*
	 * TODO: DPD (TX PA pre-distortion) + the temperature-compensation
	 * BBP trim are omitted - DPD is the TX-only calibration the RX bring-up
	 * path skips (also the fragile MCU op that can USB-re-enumerate the chip),
	 * and temp_comp is a fine BBP adjust needing the boot-up temperature read.
	 */
	rf_2a4m1_rxdc_cal(dev);
	rf_2a4m1_reg_write(dev, MT_MAC_SYS_CTRL, mac_ctrl);
	return 0;
}

static int rf_2a4m1_phy_init(struct rf_2a4m1_dev *dev)
{
	int ret;

	(void)rf_2a4m1_reg_read(dev, MT_RF_PA_MODE_CFG0);
	(void)rf_2a4m1_reg_read(dev, MT_RF_PA_MODE_CFG1);
	if ((ret = rf_2a4m1_rf_wr(dev, 0, 12, dev->ee.rf_freq_off)) < 0)
		return ret;
	if ((ret = rf_2a4m1_wr_rf_table(dev, rf_2a4m1_rf_central,
					ARRAY_SIZE(rf_2a4m1_rf_central))) < 0)
		return ret;
	if ((ret = rf_2a4m1_wr_rf_table(dev, rf_2a4m1_rf_channel,
					ARRAY_SIZE(rf_2a4m1_rf_channel))) < 0)
		return ret;
	if ((ret = rf_2a4m1_wr_rf_table(dev, rf_2a4m1_rf_vga,
					ARRAY_SIZE(rf_2a4m1_rf_vga))) < 0)
		return ret;
	return rf_2a4m1_init_cal(dev);
}

/* 2.4 GHz, 20 MHz channel program. */
static int rf_2a4m1_do_set_channel(struct rf_2a4m1_dev *dev, int channel)
{
	static const u8 freq_plan[14][4] = {
		{0x99,0x99,0x09,0x50},{0x46,0x44,0x0a,0x50},{0xec,0xee,0x0a,0x50},{0x99,0x99,0x0b,0x50},
		{0x46,0x44,0x08,0x51},{0xec,0xee,0x08,0x51},{0x99,0x99,0x09,0x51},{0x46,0x44,0x0a,0x51},
		{0xec,0xee,0x0a,0x51},{0x99,0x99,0x0b,0x51},{0x46,0x44,0x08,0x52},{0xec,0xee,0x08,0x52},
		{0x99,0x99,0x09,0x52},{0x33,0x33,0x0b,0x52},
	};
	struct rf_2a4m1_eeprom *ee = &dev->ee;
	int idx = channel - 1, i, ret;
	u32 pwr;

	if (idx < 0 || idx > 13)
		return -EINVAL;

	for (i = 0; i < 4; i++)
		if ((ret = rf_2a4m1_rf_wr(dev, 0, 17 + i, freq_plan[idx][i])) < 0)
			return ret;
	rf_2a4m1_reg_rmw(dev, MT_TX_ALC_CFG_0, 0x3f3f, ee->chan_pwr[idx] & 0x3f);
	for (i = 62; i <= 64; i++)
		rf_2a4m1_bbp_wr(dev, (u8)i, (u8)(0x37 - ee->lna_gain));

	rf_2a4m1_vco_cal(dev);
	rf_2a4m1_bbp_rmw(dev, 4, 0x18, 0);		/* bbp_set_bw(BW20) */
	if ((ret = rf_2a4m1_set_bw_filter(dev, false)) < 0)
		return ret;
	rf_2a4m1_bbp_rmw(dev, 4, 0x20, 0);		/* ch != 14 fixup */
	rf_2a4m1_bbp_wr(dev, 178, 0xff);

	pwr = (rf_2a4m1_int_to_s6(ee->ofdm_bw20[1]) << 24) |
	      (rf_2a4m1_int_to_s6(ee->ofdm_bw20[0]) << 16) |
	      (rf_2a4m1_int_to_s6(ee->cck_bw20[1]) << 8) |
	       rf_2a4m1_int_to_s6(ee->cck_bw20[0]);
	rf_2a4m1_reg_write(dev, MT_TX_PWR_CFG_0, pwr);
	dev->cur_channel = channel;
	return 0;
}

static int rf_2a4m1_write_mac_initvals(struct rf_2a4m1_dev *dev)
{
	u32 regs[4] = { 0, 0, 0, 0 };
	int i, ret;

	if ((ret = rf_2a4m1_wr_mac_table(dev, rf_2a4m1_mac_common,
					 ARRAY_SIZE(rf_2a4m1_mac_common))) < 0)
		return ret;
	if ((ret = rf_2a4m1_wr_mac_table(dev, rf_2a4m1_mac_chip,
					 ARRAY_SIZE(rf_2a4m1_mac_chip))) < 0)
		return ret;
	/* Beacon offsets: 512 B/beacon default table (offset[i] = i*8). */
	for (i = 0; i < 16; i++)
		regs[i / 4] |= (u32)(i * 8) << (8 * (i % 4));
	for (i = 0; i < 4; i++)
		rf_2a4m1_reg_write(dev, MT_BCN_OFFSET_BASE + (i << 2), regs[i]);
	rf_2a4m1_reg_write(dev, MT_AUX_CLK_CFG, 0);
	return 0;
}

static int rf_2a4m1_init_bbp(struct rf_2a4m1_dev *dev)
{
	int i, ret;

	for (i = 20; i; i--) {		/* wait_bbp_ready */
		int v = rf_2a4m1_bbp_rr(dev, 0);

		if (v > 0 && v != 0xff)
			break;
		usleep_range(1000, 1500);
	}
	if ((ret = rf_2a4m1_wr_bbp_table(dev, rf_2a4m1_bbp_common,
					 ARRAY_SIZE(rf_2a4m1_bbp_common))) < 0)
		return ret;
	return rf_2a4m1_wr_bbp_table(dev, rf_2a4m1_bbp_chip,
				     ARRAY_SIZE(rf_2a4m1_bbp_chip));
}

static int rf_2a4m1_init_wcid_mem(struct rf_2a4m1_dev *dev)
{
	const int n_wcid = 128;
	u32 z[4] = { 0, 0, 0, 0 };
	u32 *vals;
	int i, ret;

	vals = kmalloc_array(n_wcid * 2, sizeof(u32), GFP_KERNEL);
	if (!vals)
		return -ENOMEM;

	for (i = 0; i < n_wcid; i++) {
		vals[i * 2]     = 0xffffffff;
		vals[i * 2 + 1] = 0x00ffffff;
	}
	if ((ret = rf_2a4m1_mcu_burst_write(dev, MT_WCID_ADDR_BASE,
					    vals, n_wcid * 2)) < 0)
		goto out;
	if ((ret = rf_2a4m1_mcu_burst_write(dev, MT_SKEY_MODE_BASE_0, z, 4)) < 0)
		goto out;
	for (i = 0; i < n_wcid * 2; i++)
		vals[i] = 1;
	ret = rf_2a4m1_mcu_burst_write(dev, MT_WCID_ATTR_BASE, vals, n_wcid * 2);
out:
	kfree(vals);
	return ret;
}

/*
 * Compose the full bring-up: reset, USB DMA, firmware-ready confirm, Q_SELECT,
 * the MAC/BBP/RF register tables, WCID clear, the EEPROM MAC + PHY params, the
 * RF calibrations, and the channel program.  Leaves the MAC RX/TX engine
 * disabled - rf_2a4m1_mac_start_rx() enables it when the interface starts.
 */
int rf_2a4m1_chip_init(struct rf_2a4m1_dev *dev, int channel)
{
	u32 v;
	int i, stable, ret;

	/* reset CSR + BBP, then bring up USB DMA. */
	rf_2a4m1_reg_write(dev, MT_MAC_SYS_CTRL,
			   MT_MAC_SYS_CTRL_RESET_CSR | MT_MAC_SYS_CTRL_RESET_BBP);
	rf_2a4m1_reg_write(dev, MT_USB_DMA_CFG, 0);
	usleep_range(1000, 1500);
	rf_2a4m1_reg_write(dev, MT_MAC_SYS_CTRL, 0);

	v = ((u32)MT_USB_AGGR_TIMEOUT & 0xff)
	  | (((u32)MT_USB_AGGR_SIZE_LIMIT & 0xff) << 8)
	  | MT_USB_DMA_CFG_RX_BULK_EN | MT_USB_DMA_CFG_TX_BULK_EN
	  | MT_USB_DMA_CFG_RX_BULK_AGG_EN;
	rf_2a4m1_reg_write(dev, MT_USB_DMA_CFG, v);
	rf_2a4m1_reg_write(dev, MT_USB_DMA_CFG, v | MT_USB_DMA_CFG_UDMA_RX_WL_DROP);
	rf_2a4m1_reg_write(dev, MT_USB_DMA_CFG, v);

	/* Confirm the MCU is up + settled (COM_REG0 == 1 on 5 consecutive polls)
	 * before the first MCU command. */
	for (i = 0, stable = 0; i < 200 && stable < 5; i++) {
		stable = (rf_2a4m1_reg_read(dev, MT_MCU_COM_REG0) == 1) ? stable + 1 : 0;
		usleep_range(5000, 6000);
	}
	if (stable < 5) {
		dev_err(dev->dev, "firmware not ready (COM_REG0 != 1) before init\n");
		return -ETIMEDOUT;
	}

	if ((ret = rf_2a4m1_mcu_function_select(dev, 1, 1)) < 0)	/* Q_SELECT */
		return ret;
	if ((ret = rf_2a4m1_write_mac_initvals(dev)) < 0)
		return ret;
	if ((ret = rf_2a4m1_init_bbp(dev)) < 0)
		return ret;
	if ((ret = rf_2a4m1_init_wcid_mem(dev)) < 0)
		return ret;

	/* Clear beacon timers, read-clear the stat counters, US-cycle + TXOP. */
	rf_2a4m1_reg_rmw(dev, MT_BEACON_TIME_CFG, MT_BEACON_TIME_CFG_MASK, 0);
	(void)rf_2a4m1_reg_read(dev, MT_RX_STA_CNT0);
	(void)rf_2a4m1_reg_read(dev, MT_TX_STA_CNT0);
	rf_2a4m1_reg_rmw(dev, MT_US_CYC_CFG, MT_US_CYC_CNT, 0x1e);
	rf_2a4m1_reg_write(dev, MT_TXOP_CTRL_CFG, 0x3fu | (0x58u << 8));

	if ((ret = rf_2a4m1_eeprom_read(dev)) < 0)
		return ret;
	rf_2a4m1_reg_write(dev, MT_MAC_ADDR_DW0, get_unaligned_le32(dev->macaddr.a));
	rf_2a4m1_reg_write(dev, MT_MAC_ADDR_DW1,
			   (u32)dev->macaddr.a[4] | ((u32)dev->macaddr.a[5] << 8) |
			   (0xffu << 16));

	if ((ret = rf_2a4m1_phy_init(dev)) < 0)
		return ret;
	if ((ret = rf_2a4m1_do_set_channel(dev, channel)) < 0)
		return ret;

	/* RX/TX path selects + control-channel + BW20. */
	rf_2a4m1_bbp_rmw(dev, 3, 0x18, 0);
	rf_2a4m1_bbp_rmw(dev, 1, 0x18, 0);
	rf_2a4m1_reg_rmw(dev, MT_TX_BAND_CFG, 1, 0);
	rf_2a4m1_bbp_rmw(dev, 3, 0x20, 0);
	rf_2a4m1_bbp_rmw(dev, 4, 0x18, 0);

	dev->hw_inited = true;
	dev_info(dev->dev, "chip init complete (channel %d, MAC %pM)\n",
		 channel, dev->macaddr.a);
	return 0;
}

/* Enable the MAC RX/TX engine (called when the interface starts). */
static int rf_2a4m1_mac_start_rx(struct rf_2a4m1_dev *dev)
{
	rf_2a4m1_reg_write(dev, MT_MAC_SYS_CTRL, MT_MAC_SYS_CTRL_ENABLE_TX);
	rf_2a4m1_poll(dev, MT_WPDMA_GLO_CFG,
		      MT_WPDMA_GLO_CFG_TX_DMA_BUSY | MT_WPDMA_GLO_CFG_RX_DMA_BUSY, 0, 300);
	rf_2a4m1_reg_write(dev, MT_RX_FILTR_CFG,
			   MT_RX_FILTR_CFG_CRC_ERR | MT_RX_FILTR_CFG_PHY_ERR);
	rf_2a4m1_reg_write(dev, MT_MAC_SYS_CTRL,
			   MT_MAC_SYS_CTRL_ENABLE_TX | MT_MAC_SYS_CTRL_ENABLE_RX);
	rf_2a4m1_poll(dev, MT_WPDMA_GLO_CFG,
		      MT_WPDMA_GLO_CFG_TX_DMA_BUSY | MT_WPDMA_GLO_CFG_RX_DMA_BUSY, 0, 50);
	/*
	 * Read the filter back rather than trusting the write: every bit in
	 * MT_RX_FILTR_CFG DROPS a class of frames, so a stray bit here silently
	 * removes a whole frame type from the air (a beacon-shaped hole looks
	 * exactly like "no AP in range"). Expect 0x3 = drop CRC/PHY errors only.
	 */
	dev_info(dev->dev,
		 "mac rx enabled: RX_FILTR_CFG=0x%08x MAC_SYS_CTRL=0x%08x USB_DMA_CFG=0x%08x\n",
		 rf_2a4m1_reg_read(dev, MT_RX_FILTR_CFG),
		 rf_2a4m1_reg_read(dev, MT_MAC_SYS_CTRL),
		 rf_2a4m1_reg_read(dev, MT_USB_DMA_CFG));
	return 0;
}

/* ================================================================== */
/* HAL vtable (struct rf_2a4m1_hal_ops) over usbcore.                  */
/* ================================================================== */

/*
 * Bridge a received 802.11 DATA frame up into the network stack.
 *
 * A Protected DATA frame is first software-decrypted (CCMP-128) with the
 * pairwise TK the core's 4-way installed, via the core's
 * rf_2a4m1_ccmp_decrypt_rx (MIC-before-replay + 48-bit PN); the recovered
 * plaintext then takes the same path as a cleartext frame.  Deframe
 * 802.11 -> 802.3 (strip the MAC header + the RFC1042/bridge-tunnel LLC/SNAP
 * shim, rebuild an Ethernet header from DA/SA + the SNAP ethertype), then
 * eth_type_trans() + netif_rx() and account it on the netdev.
 *
 * Returns true iff the frame was consumed here (bridged or dropped): the caller
 * then does NOT hand it to the core.  Returns false for everything that must
 * stay on the core (SME) path -- all management/control frames, a Protected
 * frame arriving before the 4-way installed a key, non-SNAP data, and crucially
 * the 4-way EAPOL (ethertype 0x888E), which the core's handshake owns.
 *
 * The core carries no 802.11->802.3 helper, so this is a minimal
 * LLC/SNAP deframe done in the glue.
 */
static bool rf_2a4m1_rx_to_netdev(struct rf_2a4m1_dev *dev,
				  const u8 *data, u16 len, bool hw_decrypted)
{
	struct net_device *ndev = dev->ndev;
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)data;
	__le16 fc;
	unsigned int hdrlen;
	const u8 *snap, *payload, *da, *sa, *body;
	u16 ethertype, plen, body_len;
	unsigned int frame_len;
	struct sk_buff *skb;
	u8 *eth;
	u8 *decbuf = NULL;			/* CCMP plaintext scratch (Protected) */

	if (!ndev || len < sizeof(struct ieee80211_hdr))
		return false;
	fc = hdr->frame_control;

	/* Only DATA frames are bridged; mgmt/ctrl stay on the core (SME) path. */
	if (!ieee80211_is_data(fc))
		return false;

	hdrlen = ieee80211_hdrlen(fc);
	if (len < hdrlen)
		return false;

	if (ieee80211_has_protected(fc)) {
		/*
		 * Encrypted DATA frame. Preferred path: the chip HW-decrypted it
		 * (RXINFO.DECRYPT set, MIC verified) under the per-WCID / shared key
		 * table this driver programmed, and stripped the CCMP IV + MIC -- so
		 * the payload right after the 802.11 header is already plaintext and
		 * takes the same deframe as a cleartext frame.  Software fallback:
		 * the chip did not decrypt (no MYBSS / key-table match), so decrypt
		 * in software with the pairwise TK (dev->sme.installed_tk) + the
		 * per-STA PN replay context (dev->sme.data_rx_replay) through the
		 * core's CCMP-128 RX entry (AES-CCM MIC BEFORE the 48-bit-PN replay
		 * check).  Before the 4-way installs a key (data_key_valid == false)
		 * a Protected frame is left on the core (SME) path.
		 */
		if (hw_decrypted) {
			/*
			 * The chip verified the AES-CCM MIC and stripped the trailing
			 * MIC/ICV.  Whether it ALSO stripped the 8-octet CCMP header
			 * (IV/PN) is not fixed: mainline mt7601u (mac.c
			 * mt76_mac_process_rx) sets RX_FLAG_IV_STRIPPED on DECRYPT and
			 * clears it again only when the RXWI MT_RXINFO_PN_LEN field is
			 * non-zero.  So the plaintext LLC/SNAP sits EITHER immediately
			 * after the 802.11 header (IV stripped) OR 8 octets in (CCMP
			 * header retained).  Rather than hard-code one offset -- a wrong
			 * guess feeds the deframe garbage, the SNAP check below fails,
			 * and the frame is silently punted to the core (SME) path, which
			 * drops every data frame and stalls the whole L3 datapath --
			 * locate the plaintext by the RFC1042 LLC/SNAP signature, which
			 * is correct in both cases.
			 */
			if (len < hdrlen)
				return false;
			body     = data + hdrlen;
			body_len = len - hdrlen;
			if (body_len >= 8 + 3 &&
			    !(body[0] == 0xaa && body[1] == 0xaa && body[2] == 0x03) &&
			    body[8] == 0xaa && body[9] == 0xaa && body[10] == 0x03) {
				body     += 8;	/* CCMP header retained -> skip it */
				body_len -= 8;
			}
		} else {
			int r;

			if (!dev->sme.data_key_valid)
				return false;		/* no TK yet -> core path */

			decbuf = kmalloc(len, GFP_ATOMIC);
			if (!decbuf) {
				ndev->stats.rx_dropped++;
				return true;		/* consumed (dropped) */
			}
			r = rf_2a4m1_ccmp_decrypt_rx(data, len, hdrlen,
						     dev->sme.installed_tk,
						     decbuf, len, NULL,
						     &dev->sme.data_rx_replay);
			if (r < 0) {
				dev_dbg(dev->dev, "ccmp rx drop (%s)\n",
					r == RF_2A4M1_CCMP_RX_E_REPLAY ?
						"replay/old PN" : "MIC/malformed");
				ndev->stats.rx_dropped++;
				ndev->stats.rx_errors++;
				kfree(decbuf);
				return true;		/* consumed (dropped) */
			}
			body     = decbuf;
			body_len = (u16)r;
		}
	} else {
		body     = data + hdrlen;
		body_len = len - hdrlen;
	}

	if (body_len < 8)			/* need LLC/SNAP (6) + ethertype (2) */
		goto to_core;			/* e.g. QoS-Null: no payload -> core */

	snap = body;
	/* RFC1042 / bridge-tunnel LLC-SNAP: AA-AA-03, ethertype at [6..7]. */
	if (snap[0] != 0xaa || snap[1] != 0xaa || snap[2] != 0x03)
		goto to_core;			/* non-SNAP data -> core path */
	ethertype = ((u16)snap[6] << 8) | snap[7];

	/* DIAG: bounded log of every deframed data frame -- ethertype (0x0800 =
	 * IPv4 -> a DHCP OFFER lives here), DA class (bcast/mcast/ucast), and the
	 * HW-decrypt verdict, so an L3 stall can be pinned to "no OFFER arrived" vs
	 * "OFFER arrived and was mis-deframed". */
	{
		static atomic_t diag_n = ATOMIC_INIT(0);

		if (atomic_inc_return(&diag_n) <= 40)
			dev_info(dev->dev,
				 "rxdiag deframe: ethertype=0x%04x da=%pM %s prot=%d hwdec=%d bodylen=%u snap8=%*ph\n",
				 ethertype, ieee80211_get_DA(hdr),
				 is_broadcast_ether_addr(ieee80211_get_DA(hdr)) ? "BCAST" :
				 is_multicast_ether_addr(ieee80211_get_DA(hdr)) ? "MCAST" : "ucast",
				 ieee80211_has_protected(fc), hw_decrypted, body_len,
				 (int)min_t(unsigned int, 12u, body_len), snap);
	}

	/*
	 * DIAG: uncapped detector for an inbound DHCP server->client reply (OFFER
	 * or ACK) -- IPv4 (0x0800) UDP with source port 67 (bootps).  This is the
	 * decisive "did an OFFER arrive at all" datum, and it deliberately survives
	 * the general 40-entry deframe cap above (DHCP replies are rare, so an
	 * unbounded log here cannot flood) so it is seen across the WHOLE DHCP retry
	 * window.  Logs the DA class (a broadcast OFFER lands here group-decrypted;
	 * a unicast OFFER pairwise-decrypted) + the decrypt verdict, so a stall
	 * splits "no OFFER inbound (the AP drops our DISCOVER on decrypt)" from
	 * "OFFER arrived and was delivered (the client/route is the issue)".
	 */
	if (ethertype == ETH_P_IP && body_len >= 8 + 20 + 8) {
		const u8 *ip = snap + 8;
		unsigned int ihl = (unsigned int)(ip[0] & 0x0f) * 4;

		if ((ip[0] >> 4) == 4 && ip[9] == 17 /* IPPROTO_UDP */ &&
		    ihl >= 20 && body_len >= 8 + ihl + 8) {
			const u8 *udp = ip + ihl;
			u16 sport = get_unaligned_be16(udp);
			u16 dport = get_unaligned_be16(udp + 2);
			u16 ip_tot = get_unaligned_be16(ip + 2);	/* IP total_length */
			u16 avail  = body_len - 8;			/* delivered after SNAP */

			if (sport == 67 || dport == 68) {
				atomic_inc(&dev->rx_dhcp_reply);
				dev_info(dev->dev,
					 "rxdhcp: DHCP reply IN da=%pM %s prot=%d hwdec=%d udp %u->%u ip_total_len=%u avail=%u %s -- an OFFER/ACK reached the driver\n",
					 ieee80211_get_DA(hdr),
					 is_broadcast_ether_addr(ieee80211_get_DA(hdr)) ? "BCAST" :
					 is_multicast_ether_addr(ieee80211_get_DA(hdr)) ? "MCAST" : "ucast",
					 ieee80211_has_protected(fc), hw_decrypted,
					 sport, dport, ip_tot, avail,
					 ip_tot <= avail ? "FULL" : "TRUNCATED");
			}
		}
	}

	/*
	 * The 4-way EAPOL is a DATA frame too (ethertype 0x888E); it MUST stay on
	 * the core path -- do NOT reroute it to the stack.  (The 4-way EAPOL is
	 * cleartext; a Protected EAPOL is a group-rekey frame -- also left to the
	 * core.  TODO: the core's on-path data model uses a placeholder MIC, so
	 * encrypted group-rekey EAPOL delivery into the SME is not fully wired.)
	 */
	if (ethertype == ETH_P_PAE)
		goto to_core;

	payload = snap + 8;
	plen = body_len - 8;
	da = ieee80211_get_DA(hdr);
	sa = ieee80211_get_SA(hdr);

	skb = netdev_alloc_skb(ndev, ETH_HLEN + plen);
	if (!skb) {
		ndev->stats.rx_dropped++;
		kfree(decbuf);
		return true;			/* consumed (dropped) -- not to core */
	}
	eth = skb_put(skb, ETH_HLEN);
	memcpy(eth, da, ETH_ALEN);			/* dest */
	memcpy(eth + ETH_ALEN, sa, ETH_ALEN);		/* source */
	eth[2 * ETH_ALEN]     = snap[6];		/* ethertype, network order */
	eth[2 * ETH_ALEN + 1] = snap[7];
	if (plen)
		skb_put_data(skb, payload, plen);	/* copies out of body */

	kfree(decbuf);				/* plaintext now owned by the skb */

	frame_len = skb->len;
	skb->protocol = eth_type_trans(skb, ndev);
	ndev->stats.rx_packets++;
	ndev->stats.rx_bytes += frame_len;
	netif_rx(skb);
	return true;

to_core:
	/*
	 * Not bridged (non-SNAP / EAPOL / too short).  The core (SME) owns it, so
	 * hand the ORIGINAL frame back by returning false; free the CCMP scratch.
	 */
	kfree(decbuf);
	return false;
}

static void rf_2a4m1_rx_process_seg(struct rf_2a4m1_dev *dev,
				    u8 *seg, u16 seg_len);

/*
 * RX bottom half, in PROCESS context: drain the queued segments and deliver
 * them to the core. Sleeping is legal here, which is the whole point -- the
 * core answers a frame by calling sleeping HAL ops straight back down.
 */
static void rf_2a4m1_rx_work(struct work_struct *work)
{
	struct rf_2a4m1_dev *dev = container_of(work, struct rf_2a4m1_dev,
						rx_work);
	struct sk_buff *skb;

	while ((skb = skb_dequeue(&dev->rx_queue))) {
		if (dev->state != RF_2A4M1_STATE_REMOVING)
			rf_2a4m1_rx_process_seg(dev, skb->data, skb->len);
		kfree_skb(skb);
	}
}

/* RX URB completion (softirq): walk every DMA segment the transfer aggregated,
 * queue each for the process-context worker, then re-arm the URB. */
static void rf_2a4m1_rx_complete(struct urb *urb)
{
	struct rf_2a4m1_rx_urb *rx = urb->context;
	struct rf_2a4m1_dev *dev = rx->dev;

	if (urb->status) {
		atomic_inc(&dev->rx_urb_errs);
		atomic_set(&dev->rx_urb_last_err, urb->status);
		/*
		 * Teardown is terminal; anything else is a transient transfer
		 * error and MUST be re-armed. A URB abandoned here never returns
		 * to the ring, so N such errors silently reduce RX to nothing --
		 * a permanent, invisible failure that looks like dead air.
		 */
		if (urb->status == -ENOENT || urb->status == -ESHUTDOWN ||
		    urb->status == -ECONNRESET)
			return;
		dev_dbg(dev->dev, "rx urb status %d\n", urb->status);
		goto resubmit;
	}
	atomic_inc(&dev->rx_urbs);

	if (urb->actual_length < 40)
		goto resubmit;

	/*
	 * A bulk-IN transfer carries an AGGREGATE of DMA segments (the chip is
	 * configured with MT_USB_DMA_CFG_RX_BULK_AGG_EN), so walk every segment.
	 * Consuming only the first silently discards the rest of the transfer:
	 * the in-tree driver loops the same way (mt7601u_rx_process_entry), and
	 * an Assoc-Resp and the EAPOL-Key M1 that follows it routinely land in
	 * ONE transfer -- taking only the head drops the M1 and the 4-way stalls.
	 */
	{
		u32 off = 0, n = 0;
		bool queued = false;

		while (off + RF_2A4M1_MT_DMA_HDRS <= urb->actual_length) {
			u16 dlen = get_unaligned_le16(rx->buf + off);
			u32 slen = RF_2A4M1_MT_DMA_HDRS + dlen;
			struct sk_buff *skb;

			if (!dlen || (dlen & 0x3) ||
			    off + slen > urb->actual_length)
				break;
			n++;
			/*
			 * Copy the segment out and hand it to the work item: the
			 * URB's buffer is re-armed below and must not outlive
			 * this callback.
			 */
			skb = alloc_skb(slen, GFP_ATOMIC);
			if (skb) {
				skb_put_data(skb, rx->buf + off, slen);
				skb_queue_tail(&dev->rx_queue, skb);
				queued = true;
			}
			off += slen;
		}
		atomic_add(n, &dev->rx_segs);
		if (n > (u32)atomic_read(&dev->rx_seg_max))
			atomic_set(&dev->rx_seg_max, n);
		if (queued)
			schedule_work(&dev->rx_work);
	}

resubmit:
	if (dev->state != RF_2A4M1_STATE_REMOVING)
		usb_submit_urb(urb, GFP_ATOMIC);
}

/* Consume ONE DMA segment: unwrap the framing, parse the RXWI, then either
 * bridge a DATA frame to the stack or hand the frame up to the core (SME).
 * @seg is writable (our own copy) -- removing the L2 pad rewrites it in place. */
static void rf_2a4m1_rx_process_seg(struct rf_2a4m1_dev *dev,
				    u8 *seg, u16 seg_len)
{
	const u8 *rxwi, *payload;
	u16 rxwi_len, payload_len;
	struct rf_2a4m1_rxinfo info;
	u32 rxinfo;
	int ret;

	ret = rf_2a4m1_mt7601u_usb_rx_unwrap(seg, seg_len,
					     &rxwi, &rxwi_len,
					     &payload, &payload_len);
	if (ret != RF_2A4M1_S8021X_OK)
		return;

	rxinfo = rf_2a4m1_get_le32(rxwi + RF_2A4M1_MT_RXWI_OFF_RXINFO);

	if (rf_2a4m1_mt7601u_parse_rxwi(rxwi, rxwi_len, payload, payload_len,
					&info) == RF_2A4M1_S8021X_OK) {
		/*
		 * The RXWI MPDU_LEN is the true 802.11 frame length, excluding the
		 * L2 pad (a DMA-alignment artifact, never counted here); clamp it to
		 * the unwrapped (4-aligned) payload span.  It carries the trailing
		 * FCS only for a frame the chip did NOT hardware-decrypt (see below).
		 */
		u16 mpdu_len = (u16)((rf_2a4m1_get_le32(rxwi + RF_2A4M1_MT_RXWI_OFF_CTL)
				      & RF_2A4M1_MT_RXWI_CTL_MPDU_LEN_MASK)
				     >> RF_2A4M1_MT_RXWI_CTL_MPDU_LEN_SHIFT);
		u16 flen = min_t(u16, mpdu_len, payload_len);
		const u16 fcs_len = 4;		/* trailing 802.11 FCS */

		/*
		 * The trailing 4-B FCS is present ONLY in a frame the chip did not
		 * hardware-decrypt.  On a HW-decrypt (RXINFO.DECRYPT) the MAC strips
		 * the CCMP MIC *and* the FCS, and the RXWI MPDU_LEN already reports
		 * that shorter frame -- so subtracting 4 there truncates the real
		 * payload by 4 bytes.  That made every HW-decrypted IP datagram 4 B
		 * shorter than its own total_length, and the stack dropped each one
		 * (the DHCP OFFER, an ARP reply, the ICMP echo-reply) -- stalling the
		 * whole L3 datapath -- while the cleartext 4-way, whose frames DO
		 * carry the FCS, worked.  So strip the FCS only when NOT HW-decrypted
		 * (a cleartext or software-CCMP frame still carries it).
		 */
		if (!(rxinfo & MT_RXINFO_DECRYPT) && flen > fcs_len)
			flen -= fcs_len;

		/*
		 * Remove the hardware's L2 padding.
		 *
		 * The MAC 4-byte-aligns the payload that follows the 802.11
		 * header, so when the header is not a multiple of 4 it inserts 2
		 * bytes BETWEEN the header and the LLC/SNAP and flags it with
		 * MT_RXINFO_L2PAD. Management frames have a 24-byte header and
		 * are already aligned, so they are never padded -- but a QoS DATA
		 * header is 26 bytes and always is. Leave the pad in and every
		 * QoS data frame decodes two bytes off: the EAPOL-Key M1 that
		 * opens the 4-way is a QoS data frame, so the handshake silently
		 * never starts while beacons/auth/assoc all parse perfectly.
		 * Shift the header up over the pad (as the in-tree driver's
		 * mt76_remove_hdr_pad does) so header and payload are adjacent.
		 */
		if ((rxinfo & MT_RXINFO_L2PAD) && info.data && flen >= 2) {
			u8 *d = (u8 *)info.data;
			unsigned int hl;
			u16 fc = (u16)(d[0] | ((u16)d[1] << 8));

			hl = ieee80211_hdrlen((__force __le16)fc);
			if (flen > hl + 2 && info.len > 2) {
				memmove(d + 2, d, hl);
				info.data = d + 2;
				/*
				 * Only the BUFFER carries the 2-byte L2 pad; the RXWI
				 * MPDU_LEN (hence flen) is the true frame length and
				 * never counts it.  So flen must NOT be shortened here
				 * -- doing so truncated every HW-decrypted data frame by
				 * 2 (on top of the FCS over-strip above), the last piece
				 * that kept the DHCP OFFER / ARP reply / ICMP echo-reply
				 * 2 B below their own length so the stack dropped them.
				 * The memmove slid the header up over the pad, so the
				 * frame from info.data spans exactly flen contiguous bytes.
				 *
				 * The core (SME) path reads info.len instead -- a SEPARATE
				 * span parse_rxwi set from the padded buffer -- so it DOES
				 * drop the 2 pad bytes (the 4-way EAPOL M1 rides that path
				 * and needs the pad gone).  Keep the two consistent by
				 * adjusting info.len only.
				 */
				info.len -= 2;
			}
		}

		/*
		 * DATA frames (cleartext, or CCMP-decrypted with the installed
		 * pairwise TK) are bridged to the stack here; mgmt/control, the
		 * 4-way EAPOL, and pre-key Protected frames fall through to the
		 * core (SME) path.
		 */
		atomic_inc(&dev->rx_frames);

		/*
		 * EAPOL-Key (the 4-way) rides a DATA frame behind LLC/SNAP, so
		 * count it here where every frame passes -- the mgmt histogram
		 * cannot see it, and "no M1 arrived" vs "M1 arrived and was
		 * ignored" are opposite bugs.
		 */
		if (flen >= 2 && info.data) {
			u16 fc0 = (u16)(info.data[0] | ((u16)info.data[1] << 8));

			/*
			 * Histogram by 802.11 TYPE (0=mgmt 1=ctrl 2=data), and
			 * separately count data frames addressed to US. "We hear
			 * no data at all" and "we hear data but the AP never sent
			 * M1" are opposite root causes and the mgmt histogram
			 * cannot distinguish them.
			 */
			atomic_inc(&dev->rx_type[(fc0 >> 2) & 3]);

			if (((fc0 >> 2) & 3) == 2 && flen >= 32) {
				unsigned int hl = ieee80211_hdrlen((__force __le16)fc0);

				/* addr1 (RA) is at offset 4 for every frame. */
				if (ether_addr_equal(info.data + 4, dev->macaddr.a)) {
					atomic_inc(&dev->rx_data_to_us);
					/*
					 * Dump it. A data frame addressed to us
					 * during the 4-way is almost certainly M1;
					 * if it is not being recognised, the raw
					 * bytes say why (wrong header length, a
					 * Protected bit we did not expect, an
					 * unexpected LLC) where a counter cannot.
					 */
					if (atomic_inc_return(&dev->rx_d2u_logged) <= 6)
						dev_info(dev->dev,
							 "rx DATA to us: fc=0x%04x hdrlen=%u prot=%d len=%u body=%*ph\n",
							 fc0, hl,
							 !!(fc0 & 0x4000), flen,
							 (int)min_t(unsigned int, 16u, flen - hl),
							 info.data + hl);
				} else if (info.data[4] & 0x01) {
					/*
					 * DIAG: GROUP-addressed (bcast/mcast) data frame --
					 * a broadcast DHCP OFFER to an IP-less client lands
					 * here. Log the HW-decrypt verdict (DECRYPT set? MIC
					 * err?) + the body bytes so a stall can be pinned to
					 * "the OFFER arrived group-encrypted and the GTK/SKEY
					 * path did not decrypt it" (multicast=0 at netdev).
					 */
					static atomic_t grp_n = ATOMIC_INIT(0);

					if (atomic_inc_return(&grp_n) <= 20)
						dev_info(dev->dev,
							 "rx GROUP data: da=%pM fc=0x%04x prot=%d rxinfo=0x%08x DECRYPT=%d MICERR=%d body=%*ph\n",
							 info.data + 4, fc0, !!(fc0 & 0x4000),
							 rxinfo, !!(rxinfo & MT_RXINFO_DECRYPT),
							 !!(rxinfo & (MT_RXINFO_ICVERR | MT_RXINFO_MICERR)),
							 (int)min_t(unsigned int, 16u, flen - hl),
							 info.data + hl);
				}
				if (flen >= hl + 8 &&
				    get_unaligned_be16(info.data + hl + 6) == 0x888e)
					atomic_inc(&dev->rx_eapol);
			}
		}

		if (flen >= 2 && info.data) {
			u16 fc = (u16)(info.data[0] | ((u16)info.data[1] << 8));

			if (((fc >> 2) & 3) == 0) {	/* 802.11 mgmt */
				u8 sub = (fc >> 4) & 0xf;

				atomic_inc(&dev->rx_mgmt_sub[sub]);
				/*
				 * The AP's VERDICT on us, in its own words. A
				 * counted assoc-response says only that the AP
				 * replied -- not that it said yes; and a deauth's
				 * reason code names exactly why it threw us off
				 * (e.g. 15 = 4-way timeout). Without these two
				 * fields "associated" is an assumption.
				 * Mgmt body starts at 24: assoc-resp is
				 * cap(2) status(2) aid(2); deauth is reason(2).
				 */
				if (sub == 1 && flen >= 30)
					dev_info(dev->dev,
						 "rx assoc-resp: status=%u aid=0x%04x from %pM\n",
						 get_unaligned_le16(info.data + 26),
						 get_unaligned_le16(info.data + 28),
						 info.data + 10);
				else if (sub == 12 && flen >= 26)
					dev_info(dev->dev,
						 "rx DEAUTH: reason=%u from %pM\n",
						 get_unaligned_le16(info.data + 24),
						 info.data + 10);
				else if (sub == 11 && flen >= 30)
					dev_info(dev->dev,
						 "rx auth: alg=%u seq=%u status=%u from %pM\n",
						 get_unaligned_le16(info.data + 24),
						 get_unaligned_le16(info.data + 26),
						 get_unaligned_le16(info.data + 28),
						 info.data + 10);
				/* Was it the AP we were actually asked to join?
				 * SA = addr2, at offset 10 of the mgmt header. */
				if (flen >= 16 &&
				    ether_addr_equal(info.data + 10,
						     dev->connect_bssid)) {
					if (sub == 5)
						atomic_inc(&dev->rx_proberesp_target);
					else if (sub == 11)
						atomic_inc(&dev->rx_auth_target);
				}
				/*
				 * Capture the connect target's beacon (8) /
				 * probe-response (5) so the connect completion can
				 * publish its BSS to cfg80211 with the AP's real IEs
				 * before reporting success -- a FullMAC driver must
				 * announce the AP (a scan or an inform_bss) first, or
				 * cfg80211 warns. Record the signal on every match and
				 * the raw frame whenever it fits the fixed buffer.
				 */
				if ((sub == 8 || sub == 5) &&
				    dev->connect_pending && flen >= 16 &&
				    ether_addr_equal(info.data + 10,
						     dev->connect_bssid)) {
					spin_lock(&dev->bss_frame_lock);
					dev->bss_frame_rssi = info.rssi;
					if (flen >= 36 &&
					    flen <= sizeof(dev->bss_frame)) {
						memcpy(dev->bss_frame,
						       info.data, flen);
						dev->bss_frame_len = flen;
					}
					spin_unlock(&dev->bss_frame_lock);
				}
				/*
				 * Scan harvest: while a scan is dwelling on this
				 * channel, publish EVERY beacon / probe-response
				 * to cfg80211's BSS table (not just the connect
				 * target) -- this is what feeds `iw scan` and
				 * SSID-driven connect.  A no-op when no scan runs.
				 */
				if (sub == 8 || sub == 5)
					rf_2a4m1_scan_rx_bss(dev, info.data,
							     flen, info.rssi);
				/*
				 * Beacon / probe-response detail, incl. RSSI --
				 * dev_dbg, not dev_info: in a busy 2.4 GHz cell
				 * this fires many times per second. Enable via
				 * dynamic debug when diagnosing why a connect
				 * never leaves the scan state.
				 */
				if (sub == 8 || sub == 5) {
					/*
					 * Log the first few with the SOURCE BSSID: which AP
					 * we hear is the only direct evidence of the channel
					 * the radio is REALLY on -- a retune that silently
					 * does not take effect still delivers beacons, just
					 * from the old channel's APs. Bounded so a busy cell
					 * cannot flood the log.
					 */
					if (atomic_inc_return(&dev->rx_bcn_logged) <= 10)
						dev_info(dev->dev,
							 "rx mgmt sub=%u rssi=%d snr=%d mcs=%u from %pM\n",
							 sub, info.rssi, info.snr,
							 info.mcs,
							 flen >= 16 ? info.data + 10 : info.data);
					else
						dev_dbg(dev->dev,
							"rx mgmt sub=%u rssi=%d snr=%d mcs=%u from %pM\n",
							sub, info.rssi, info.snr,
							info.mcs,
							flen >= 16 ? info.data + 10 : info.data);
				}
			}
		}

		/*
		 * HW-decrypt status from the RXWI rxinfo word: the chip decrypted
		 * this frame iff DECRYPT is set and neither ICV nor MIC error. Count
		 * the encrypted data plane so the L3 round-trip can prove the traffic
		 * was HW-crypted (not software CCMP).
		 */
		{
			bool hw_dec = (rxinfo & MT_RXINFO_DECRYPT) &&
				      !(rxinfo & (MT_RXINFO_ICVERR |
						  MT_RXINFO_MICERR));

			if (flen >= 2 && info.data &&
			    ((info.data[0] >> 2) & 3) == 2) {	/* data frame */
				if (info.data[1] & 0x40)	/* FC.Protected */
					atomic_inc(&dev->rx_data_protected);
				if (rxinfo & (MT_RXINFO_ICVERR | MT_RXINFO_MICERR))
					atomic_inc(&dev->rx_mic_err);
				if (hw_dec &&
				    atomic_inc_return(&dev->rx_data_decrypt_ok) == 1)
					dev_info(dev->dev,
						 "hwcrypto: first HW-decrypted RX data frame (RXINFO.DECRYPT set, MIC OK) -- chip decrypt path live\n");
			}

			if (!rf_2a4m1_rx_to_netdev(dev, info.data, flen, hw_dec))
				rf_2a4m1_hal_deliver_rx(&dev->hal, &info);
		}
	}
}

int rf_2a4m1_usb_rx_start(struct rf_2a4m1_dev *dev)
{
	unsigned int pipe = usb_rcvbulkpipe(dev->udev,
			dev->in_eps[RF_2A4M1_EP_IN_RX] & USB_ENDPOINT_NUMBER_MASK);
	int i, ret;

	for (i = 0; i < RF_2A4M1_RX_URBS; i++) {
		struct rf_2a4m1_rx_urb *rx = &dev->rx[i];

		rx->dev = dev;
		rx->buf = kmalloc(RF_2A4M1_RX_BUF_SZ, GFP_KERNEL);
		rx->urb = usb_alloc_urb(0, GFP_KERNEL);
		if (!rx->buf || !rx->urb) {
			ret = -ENOMEM;
			goto err;
		}
		usb_fill_bulk_urb(rx->urb, dev->udev, pipe,
				  rx->buf, RF_2A4M1_RX_BUF_SZ,
				  rf_2a4m1_rx_complete, rx);
		ret = usb_submit_urb(rx->urb, GFP_KERNEL);
		if (ret)
			goto err;
	}
	return 0;
err:
	rf_2a4m1_usb_rx_stop(dev);
	return ret;
}

void rf_2a4m1_usb_rx_stop(struct rf_2a4m1_dev *dev)
{
	int i;

	for (i = 0; i < RF_2A4M1_RX_URBS; i++) {
		struct rf_2a4m1_rx_urb *rx = &dev->rx[i];

		if (rx->urb) {
			usb_kill_urb(rx->urb);
			usb_free_urb(rx->urb);
			rx->urb = NULL;
		}
		kfree(rx->buf);
		rx->buf = NULL;
	}
	/*
	 * Order matters. The URBs are dead, so nothing new can be queued; now
	 * flush the worker (which may still be delivering -- and delivering can
	 * radiate), then wait out the TX URBs it submitted, then drop whatever
	 * never got delivered.
	 */
	cancel_work_sync(&dev->rx_work);
	usb_kill_anchored_urbs(&dev->tx_anchor);
	skb_queue_purge(&dev->rx_queue);
	dev->rx_running = false;
}

/*
 * Idempotent RX bring-up.  Enable the MAC RX/TX engine (if the chip came up) +
 * start the TX-status drain + arm the bulk-IN ring -- but only once.  A scan
 * needs RX live before any connect starts it (a standalone `iw scan` before an
 * association must still hear beacons), and a later connect-time hal_start then
 * finds the ring already up and no-ops it here.  rf_2a4m1_usb_rx_start() is NOT
 * idempotent (it re-kmallocs + re-submits every URB), so the rx_running guard is
 * what makes a second caller safe.
 */
int rf_2a4m1_usb_rx_ensure(struct rf_2a4m1_dev *dev)
{
	int ret;

	if (dev->rx_running)
		return 0;
	if (dev->hw_inited)
		rf_2a4m1_mac_start_rx(dev);
	rf_2a4m1_usb_stat_start(dev);
	ret = rf_2a4m1_usb_rx_start(dev);
	if (ret)
		return ret;
	dev->rx_running = true;
	return 0;
}

/* --- ops --- */

/*
 * Program the MT7601U HW crypto key table from the completed 4-way's derived
 * keys, so the chip does the CCMP data plane in hardware (native offload) rather
 * than the software route.  Mirrors the mainline mt7601u key setup:
 *   - pairwise TK -> per-WCID key table (MT_WCID_ADDR/KEY/IV/ATTR at slot
 *     RF_2A4M1_HW_WCID): the chip HW-encrypts a TX frame whose TXWI.wcid == slot
 *     (WIV=0) and HW-decrypts a Protected RX frame whose TA matches the CAM;
 *   - group GTK   -> shared key table (MT_SKEY / MT_SKEY_MODE, BSS 0, key ids
 *     1 + 2): the RX engine decrypts group-addressed frames indexed by the KeyID
 *     in the received CCMP header, so program both candidate CCMP group ids;
 *   - BSSID       -> MT_MAC_BSSID (the MYBSS filter the RX crypto engine needs
 *     before it will look up the key and set RXINFO.DECRYPT).
 * The keys are read from the in-driver SME (dev->sme), which runs the 4-way
 * itself -- there is no wpa_supplicant add_key on this FullMAC path, so the glue
 * installs them here, at connect completion (process context: reg writes sleep).
 */
int rf_2a4m1_usb_install_hw_keys(struct rf_2a4m1_dev *dev)
{
	const u8 idx = RF_2A4M1_HW_WCID;
	const u8 *bssid = dev->sme.peer.a;
	const u8 *tk = dev->sme.installed_tk;
	const u8 *gtk = dev->sme.gtk;
	u32 attr;
	int i, gid;

	if (!dev->hw_inited)
		return -ENODEV;
	if (!dev->sme.ptk_installed || !dev->sme.data_key_valid)
		return -EINVAL;

	/* Pairwise TK -> WCID key slot (unicast HW CCMP TX + RX). */
	rf_2a4m1_reg_write(dev, MT_WCID_ADDR(idx), get_unaligned_le32(bssid));
	rf_2a4m1_reg_write(dev, MT_WCID_ADDR(idx) + 4,
			   (u32)bssid[4] | ((u32)bssid[5] << 8));
	for (i = 0; i < 4; i++)			/* 16-B TK in the low half ... */
		rf_2a4m1_reg_write(dev, MT_WCID_KEY(idx) + i * 4,
				   get_unaligned_le32(tk + i * 4));
	for (i = 4; i < 8; i++)			/* ... high half zeroed */
		rf_2a4m1_reg_write(dev, MT_WCID_KEY(idx) + i * 4, 0);
	rf_2a4m1_reg_write(dev, MT_WCID_IV(idx), 0x20000001u);	/* ExtIV, KeyID 0 */
	rf_2a4m1_reg_write(dev, MT_WCID_IV(idx) + 4, 0);
	attr = MT_WCID_ATTR_PAIRWISE |
	       ((u32)(MT_CIPHER_AES_CCMP & 7) << MT_WCID_ATTR_PKEY_MODE_SHIFT);
	rf_2a4m1_reg_write(dev, MT_WCID_ATTR(idx), attr);
	rf_2a4m1_reg_rmw(dev, MT_WCID_DROP(idx), MT_WCID_DROP_MASK(idx), 0);

	/* Group GTK -> shared key table, BSS 0, both candidate CCMP key ids. */
	for (gid = 1; gid <= 2; gid++) {
		u32 mode;

		for (i = 0; i < 4; i++)
			rf_2a4m1_reg_write(dev, MT_SKEY(0, gid) + i * 4,
					   get_unaligned_le32(gtk + i * 4));
		for (i = 4; i < 8; i++)
			rf_2a4m1_reg_write(dev, MT_SKEY(0, gid) + i * 4, 0);
		mode = rf_2a4m1_reg_read(dev, MT_SKEY_MODE_0(0));
		mode &= ~(MT_SKEY_MODE_MASK << MT_SKEY_MODE_SHIFT(0, gid));
		mode |= (u32)MT_CIPHER_AES_CCMP << MT_SKEY_MODE_SHIFT(0, gid);
		rf_2a4m1_reg_write(dev, MT_SKEY_MODE_0(0), mode);
	}

	/* BSSID -> MYBSS filter (the RX crypto engine's gate). */
	rf_2a4m1_reg_write(dev, MT_MAC_BSSID_DW0, get_unaligned_le32(bssid));
	rf_2a4m1_reg_write(dev, MT_MAC_BSSID_DW1,
			   (u32)bssid[4] | ((u32)bssid[5] << 8));

	ether_addr_copy(dev->hw_peer, bssid);
	dev->hw_wcid = idx;
	dev->hw_key_installed = true;

	dev_info(dev->dev,
		 "hwcrypto: CCMP key table programmed - WCID %u peer %pM attr=0x%08x (rb 0x%08x) key0=0x%08x skey_mode=0x%08x; chip HW-encrypts TX + HW-decrypts RX\n",
		 idx, bssid, attr, rf_2a4m1_reg_read(dev, MT_WCID_ATTR(idx)),
		 rf_2a4m1_reg_read(dev, MT_WCID_KEY(idx)),
		 rf_2a4m1_reg_read(dev, MT_SKEY_MODE_0(0)));
	return 0;
}

/*
 * Transmit an 802.3 frame as an HW-CCMP-encrypted 802.11 Data frame.  Build the
 * real 24-octet 802.11 Data header (toDS: A1=BSSID, A2=us, A3=DA) + the RFC1042
 * LLC/SNAP shim, set FC.Protected, and hand the chip PLAINTEXT with TXWI.wcid =
 * the pairwise key slot and DMA WIV=0 (op_tx derives WIV from key_slot != 0xff) --
 * so the MT7601U CCMP engine inserts the CCMP header (its own PN counter) +
 * encrypts + appends the 8-B MIC.  Atomic-context safe (GFP_ATOMIC + async
 * bulk-out): ndo_start_xmit can run in softirq.  Returns 0 when queued.
 */
int rf_2a4m1_usb_hw_data_tx(struct rf_2a4m1_dev *dev, const u8 *eth,
			    unsigned int eth_len)
{
	struct rf_2a4m1_mpdu m;
	struct rf_2a4m1_tx_params tp;
	const u8 *da, *payload;
	unsigned int plen, o;
	u16 ethertype, sc;
	u8 *frame;
	int ret;

	if (!dev->hw_inited || !dev->hw_key_installed)
		return -ENODEV;
	if (eth_len < ETH_HLEN)
		return -EINVAL;
	plen = eth_len - ETH_HLEN;
	/* 24 hdr + 8 LLC/SNAP + payload + TXWI + DMA framing under the ATOMIC buf. */
	if (24 + 8 + plen + RF_2A4M1_MT7601U_TXWI_SIZE + 8 > RF_2A4M1_TX_BUF_SZ)
		return -EMSGSIZE;

	da        = eth;			/* 802.3 dest -> A3 (DA) */
	ethertype = ((u16)eth[12] << 8) | eth[13];
	payload   = eth + ETH_HLEN;

	frame = kmalloc(24 + 8 + plen, GFP_ATOMIC);
	if (!frame)
		return -ENOMEM;

	frame[0] = 0x08;			/* Type=Data(2), subtype 0 */
	frame[1] = 0x01 | 0x40;			/* toDS + Protected */
	frame[2] = 0; frame[3] = 0;		/* Duration -- HW fills it */
	ether_addr_copy(frame + 4,  dev->hw_peer);	/* A1 = RA = BSSID */
	ether_addr_copy(frame + 10, dev->macaddr.a);	/* A2 = TA = us    */
	ether_addr_copy(frame + 16, da);		/* A3 = DA         */
	sc = (u16)(dev->data_seq++ << 4);		/* seqno << 4, frag 0 */
	put_unaligned_le16(sc, frame + 22);
	o = 24;
	frame[o++] = 0xaa; frame[o++] = 0xaa; frame[o++] = 0x03;	/* LLC/SNAP */
	frame[o++] = 0x00; frame[o++] = 0x00; frame[o++] = 0x00;
	frame[o++] = (u8)(ethertype >> 8);
	frame[o++] = (u8)(ethertype & 0xff);
	if (plen)
		memcpy(frame + o, payload, plen);
	o += plen;

	memset(&tp, 0, sizeof(tp));
	tp.wcid     = dev->hw_wcid;	/* TXWI.wcid = pairwise slot */
	tp.key_slot = dev->hw_wcid;	/* != 0xff => WIV=0 => chip encrypts */
	tp.ac       = 1;		/* AC_BE */
	tp.gen      = RF_2A4M1_HAL_GEN_LEGACY;
	tp.phy_mode = RF_2A4M1_HAL_PHY_OFDM;	/* OFDM 6 Mbps -- robust */
	tp.bw_mhz   = 20;
	tp.n_ss     = 1;
	/* Stamp the encrypted-class pktid so the MAC posts a TX-status report we
	 * can read back: this is the frame whose ACK/no-ACK is the decisive datum
	 * (does the AP receive our HW-encrypted data frame?). */
	tp.pktid    = RF_2A4M1_PID_ENCRYPTED;

	m.data = frame;
	m.len  = (u16)o;
	m.cap  = (u16)o;
	/* op_tx copies @frame into its own DMA buffer, so free @frame after. */
	ret = dev->hal.ops->tx(&dev->hal, &m, &tp);
	kfree(frame);
	if (!ret && atomic_inc_return(&dev->tx_data_ccmp) == 1)
		dev_info(dev->dev,
			 "hwcrypto: first HW-CCMP data frame TX'd (%u B plaintext MPDU, FC.Protected=1, wcid=%u, WIV=0 -> chip encrypts)\n",
			 (unsigned int)o, dev->hw_wcid);
	/* DIAG: bounded per-frame TX log -- ethertype (0x0800 = the DHCP DISCOVER),
	 * DA (bcast for DISCOVER), length, and the tx return code, so an L3 stall
	 * can be pinned to "the DISCOVER never radiated" vs "it radiated + the AP
	 * did not answer". */
	{
		static atomic_t tx_diag_n = ATOMIC_INIT(0);

		if (atomic_inc_return(&tx_diag_n) <= 16)
			dev_info(dev->dev,
				 "txdiag: ethertype=0x%04x da=%pM %s mpdu=%uB rc=%d\n",
				 ethertype, da,
				 is_broadcast_ether_addr(da) ? "BCAST" :
				 is_multicast_ether_addr(da) ? "MCAST" : "ucast",
				 (unsigned int)o, ret);
	}
	return ret;
}

/* ================================================================== */
/* TX-status (MT_TX_STAT_FIFO) reading -- the decisive data-plane      */
/* measurement: does the AP ACK our frames?                           */
/*                                                                    */
/* rc=0 from the bulk-out is only DMA-enqueue success. To learn if a   */
/* frame reached + was ACK'd by the AP we must read the MAC's TX-status */
/* report FIFO. The MAC posts a report per frame stamped with a nonzero */
/* TXWI pktid; each report carries VALID + SUCCESS(ACK'd) + ACKREQ +    */
/* WCID + the pktid. A glue-side delayed work drains the FIFO in process */
/* context (the vendor-control reg read sleeps) and buckets each report */
/* by pktid, so ACK success is split per frame class:                  */
/*   pktid 1 CLEARTEXT  -- WIV=1 connect mgmt/EAPOL (known-good control) */
/*   pktid 2 ENCRYPTED  -- WIV=0 HW-CCMP data frame to the AP (the ask)  */
/*   pktid 3 BOGUS      -- WIV=0 to a bogus BSSID (success MUST be 0)    */
/* Follows the mainline mt7601u report-FIFO drain                       */
/* (mac.c mt7601u_mac_fetch_tx_status()).                              */
/* ================================================================== */

/* Stamp a 4-bit pktid into the built TXWI's len_ctl (bits 15:12). */
static void rf_2a4m1_txwi_set_pktid(u8 *txwi, u8 pktid)
{
	u16 lenctl = get_unaligned_le16(txwi + MT_TXWI_OFF_LEN);

	lenctl &= ~MT_TXWI_LEN_PKTID_MASK;
	lenctl |= ((u16)pktid << MT_TXWI_LEN_PKTID_SHIFT) & MT_TXWI_LEN_PKTID_MASK;
	put_unaligned_le16(lenctl, txwi + MT_TXWI_OFF_LEN);
}

/*
 * Drain the MAC TX-status report FIFO, bucketing each report by pktid.  Reading
 * MT_TX_STAT_FIFO pops the oldest report; stop at the first slot with VALID
 * clear (or a read error).  Process context only (reg_read is a sleeping vendor
 * control transfer).  Bounded per call so a wedged FIFO cannot spin.
 */
static void rf_2a4m1_drain_tx_status(struct rf_2a4m1_dev *dev)
{
	int i;

	for (i = 0; i < 24; i++) {
		u32 fifo = rf_2a4m1_reg_read(dev, MT_TX_STAT_FIFO);
		bool success, ackreq, aggr;
		u8 pid, wcid;

		if (fifo == ~0u || !(fifo & MT_TXS_FIFO_VALID))
			break;			/* read error or empty slot */

		pid     = (fifo & MT_TXS_FIFO_PID_MASK) >> MT_TXS_FIFO_PID_SHIFT;
		wcid    = (fifo & MT_TXS_FIFO_WCID_MASK) >> MT_TXS_FIFO_WCID_SHIFT;
		success = !!(fifo & MT_TXS_FIFO_SUCCESS);
		ackreq  = !!(fifo & MT_TXS_FIFO_ACKREQ);
		aggr    = !!(fifo & MT_TXS_FIFO_AGGR);

		atomic_inc(&dev->txs_reports);
		atomic_inc(&dev->txs_valid[pid]);
		if (success)
			atomic_inc(&dev->txs_success[pid]);
		if (ackreq)
			atomic_inc(&dev->txs_ackreq[pid]);

		/* Verbatim per-report line, bounded. pid: 1=CLEARTEXT(WIV=1)
		 * 2=ENCRYPTED(WIV=0 to AP) 3=BOGUS(WIV=0 to bogus BSSID). */
		if (atomic_inc_return(&dev->txs_logged) <= 64)
			dev_info(dev->dev,
				 "txstat: fifo=0x%08x valid=1 success=%d ackreq=%d aggr=%d wcid=%u pid=%u rate=0x%04x\n",
				 fifo, success, ackreq, aggr, wcid, pid,
				 (fifo >> MT_TXS_FIFO_RATE_SHIFT) & 0xffff);
	}
}

static void rf_2a4m1_stat_worker(struct work_struct *w)
{
	struct rf_2a4m1_dev *dev =
		container_of(to_delayed_work(w), struct rf_2a4m1_dev, stat_work);

	if (!dev->stat_polling || dev->state == RF_2A4M1_STATE_REMOVING)
		return;
	if (dev->hw_inited)
		rf_2a4m1_drain_tx_status(dev);
	if (dev->stat_polling)
		schedule_delayed_work(&dev->stat_work,
				      msecs_to_jiffies(RF_2A4M1_STAT_POLL_MS));
}

void rf_2a4m1_usb_stat_init(struct rf_2a4m1_dev *dev)
{
	INIT_DELAYED_WORK(&dev->stat_work, rf_2a4m1_stat_worker);
}

void rf_2a4m1_usb_stat_start(struct rf_2a4m1_dev *dev)
{
	if (dev->stat_polling)
		return;
	dev->stat_polling = true;
	schedule_delayed_work(&dev->stat_work,
			      msecs_to_jiffies(RF_2A4M1_STAT_POLL_MS));
}

void rf_2a4m1_usb_stat_stop(struct rf_2a4m1_dev *dev)
{
	dev->stat_polling = false;
	cancel_delayed_work_sync(&dev->stat_work);
}

/* Final drain + per-class summary. The decisive line of the whole run. */
void rf_2a4m1_usb_stat_report(struct rf_2a4m1_dev *dev)
{
	if (dev->hw_inited)
		rf_2a4m1_drain_tx_status(dev);		/* sweep trailing reports */
	dev_info(dev->dev,
		 "txstat summary: total_reports=%d | CLEARTEXT(pid=1,WIV=1): valid=%d ackreq=%d success=%d | ENCRYPTED(pid=2,WIV=0->AP): valid=%d ackreq=%d success=%d | BOGUS(pid=3,WIV=0->bogus): valid=%d ackreq=%d success=%d\n",
		 atomic_read(&dev->txs_reports),
		 atomic_read(&dev->txs_valid[RF_2A4M1_PID_CLEARTEXT]),
		 atomic_read(&dev->txs_ackreq[RF_2A4M1_PID_CLEARTEXT]),
		 atomic_read(&dev->txs_success[RF_2A4M1_PID_CLEARTEXT]),
		 atomic_read(&dev->txs_valid[RF_2A4M1_PID_ENCRYPTED]),
		 atomic_read(&dev->txs_ackreq[RF_2A4M1_PID_ENCRYPTED]),
		 atomic_read(&dev->txs_success[RF_2A4M1_PID_ENCRYPTED]),
		 atomic_read(&dev->txs_valid[RF_2A4M1_PID_BOGUS]),
		 atomic_read(&dev->txs_ackreq[RF_2A4M1_PID_BOGUS]),
		 atomic_read(&dev->txs_success[RF_2A4M1_PID_BOGUS]));
}

/*
 * Falsifiable control for the TX-status measurement.  Send a few HW-encrypted
 * (WIV=0, wcid = pairwise slot) data frames whose RA (A1) is a bogus,
 * locally-administered BSSID no device on the channel owns.  No receiver can
 * ACK them, so their TX-status SUCCESS must be 0 -- which proves a SUCCESS=1 on
 * the real path is a genuine ACK, not a stuck bit.  A falsifiable control for
 * the ACK measurement.  One-shot; process context.
 */
void rf_2a4m1_usb_bogus_bssid_probe(struct rf_2a4m1_dev *dev)
{
	static const u8 bogus[RF_2A4M1_ETH_ALEN] = {
		0x02, 0x00, 0x00, 0xde, 0xad, 0x01
	};
	struct rf_2a4m1_tx_params tp;
	struct rf_2a4m1_mpdu m;
	unsigned int o;
	u8 *frame;
	int i;

	if (!dev->hw_inited || !dev->hw_key_installed)
		return;
	if (atomic_cmpxchg(&dev->bogus_probe_done, 0, 1) != 0)
		return;					/* one-shot */

	frame = kmalloc(32, GFP_KERNEL);
	if (!frame)
		return;
	/* Minimal 802.11 Data frame, toDS + Protected: A1 = bogus RA, A2 = us,
	 * A3 = bogus DA, + an 8-B LLC/SNAP shim (no payload).  The chip encrypts
	 * under the pairwise key (TXWI.wcid = hw slot, WIV=0) regardless of A1. */
	frame[0] = 0x08;			/* Data, subtype 0 */
	frame[1] = 0x01 | 0x40;			/* toDS + Protected */
	frame[2] = 0; frame[3] = 0;
	ether_addr_copy(frame + 4,  bogus);		/* A1 = RA = bogus  */
	ether_addr_copy(frame + 10, dev->macaddr.a);	/* A2 = TA = us     */
	ether_addr_copy(frame + 16, bogus);		/* A3 = DA = bogus  */
	put_unaligned_le16((u16)(dev->data_seq++ << 4), frame + 22);
	o = 24;
	frame[o++] = 0xaa; frame[o++] = 0xaa; frame[o++] = 0x03;
	frame[o++] = 0x00; frame[o++] = 0x00; frame[o++] = 0x00;
	frame[o++] = 0x08; frame[o++] = 0x00;	/* ethertype IPv4 (arbitrary) */

	memset(&tp, 0, sizeof(tp));
	tp.wcid     = dev->hw_wcid;
	tp.key_slot = dev->hw_wcid;		/* != 0xff => WIV=0 => chip encrypts */
	tp.ac       = 1;			/* AC_BE */
	tp.gen      = RF_2A4M1_HAL_GEN_LEGACY;
	tp.phy_mode = RF_2A4M1_HAL_PHY_OFDM;
	tp.bw_mhz   = 20;
	tp.n_ss     = 1;
	tp.pktid    = RF_2A4M1_PID_BOGUS;

	m.data = frame;
	m.len  = (u16)o;
	m.cap  = (u16)o;
	for (i = 0; i < 4; i++)			/* a few, so one loss is not the result */
		dev->hal.ops->tx(&dev->hal, &m, &tp);
	kfree(frame);
	dev_info(dev->dev,
		 "txstat: bogus-BSSID control -- sent 4x HW-encrypted frames to %pM (pktid=%u); TX-status SUCCESS must be 0\n",
		 bogus, RF_2A4M1_PID_BOGUS);
}

static int rf_2a4m1_op_start(struct rf_2a4m1_hal *h,
			     const struct rf_2a4m1_hal_cfg *cfg)
{
	struct rf_2a4m1_dev *dev = h->priv;

	(void)cfg;
	/*
	 * The MAC/BBP/RF register init + calibration + channel program run once
	 * at probe (rf_2a4m1_chip_init).  Enable the MAC RX/TX engine (only if the
	 * chip actually came up), start draining the TX-status FIFO (so cleartext
	 * connect/EAPOL ACKs are captured alongside the later encrypted data-frame
	 * ACKs), and arm the bulk-IN RX ring -- all idempotent, so a scan that
	 * already brought RX up before this connect-time hal_start is a no-op here.
	 * The core's inline hal_start has already registered the SME rx_cb, so
	 * connect's RX delivery stays wired regardless.
	 */
	return rf_2a4m1_usb_rx_ensure(dev);
}

static void rf_2a4m1_op_stop(struct rf_2a4m1_hal *h)
{
	struct rf_2a4m1_dev *dev = h->priv;

	rf_2a4m1_usb_stat_stop(dev);
	rf_2a4m1_usb_rx_stop(dev);
}

static int rf_2a4m1_op_set_channel(struct rf_2a4m1_hal *h,
				   const struct rf_2a4m1_chan_def *c)
{
	struct rf_2a4m1_dev *dev = h->priv;
	int ch;

	if (!dev->hw_inited || !dev->ee.valid)
		return -ENODEV;
	/* 2.4 GHz: center_mhz 2412..2472 -> channel 1..13. */
	ch = ((int)c->center_mhz - 2407) / 5;
	if (ch < 1 || ch > 13)
		return -EINVAL;
	return rf_2a4m1_do_set_channel(dev, ch);
}

/* TX: build the TXWI + wrap in the DMA frame (pure funcs), then
 * bulk-out on the EDCA data endpoint. */
static int rf_2a4m1_op_tx(struct rf_2a4m1_hal *h, struct rf_2a4m1_mpdu *m,
			  const struct rf_2a4m1_tx_params *tp)
{
	struct rf_2a4m1_dev *dev = h->priv;
	u8 txwi[RF_2A4M1_MT7601U_TXWI_SIZE];
	u16 out_len = 0;
	u8 *frame;
	int ret;
	bool wiv = (tp->key_slot == 0xff);

	ret = rf_2a4m1_mt7601u_build_txwi(txwi, tp, m);
	if (ret != RF_2A4M1_S8021X_OK)
		return -EINVAL;

	/*
	 * TX-status correlation: the MAC only posts a report to MT_TX_STAT_FIFO
	 * for a frame whose TXWI pktid is nonzero.  The encrypted (WIV=0) and
	 * bogus classes carry their pktid via tp->pktid (already stamped by
	 * build_txwi above); tag the remaining cleartext (WIV=1) class -- the
	 * connect mgmt/EAPOL frames the core sends with tp->pktid == 0 -- so its
	 * ACK success is measured too.  The stat_work drain buckets by this pktid.
	 * pktid lives in the TXWI (a host<->chip descriptor); it never goes on air,
	 * so the frame the AP sees is unchanged.
	 */
	if (tp->pktid == 0)
		rf_2a4m1_txwi_set_pktid(txwi, wiv ? RF_2A4M1_PID_CLEARTEXT :
						   RF_2A4M1_PID_ENCRYPTED);

	/*
	 * Count EVERY frame we hand the radio, and record its length. A valid
	 * 802.11 management frame is >= 24 B of header alone, so a short MPDU
	 * here means we are radiating something no AP can parse.
	 */
	atomic_inc(&dev->tx_calls);
	if (m->data && m->len)
		atomic_set(&dev->tx_last_len, m->len);

	/*
	 * GFP_ATOMIC: the core calls this from the RX completion path (softirq),
	 * not just from the connect work -- a sleeping allocation here is a
	 * "scheduling while atomic" panic.
	 */
	frame = kmalloc(RF_2A4M1_TX_BUF_SZ, GFP_ATOMIC);
	if (!frame)
		return -ENOMEM;

	ret = rf_2a4m1_mt7601u_usb_tx_wrap(frame, RF_2A4M1_TX_BUF_SZ,
					   txwi, RF_2A4M1_MT7601U_TXWI_SIZE,
					   m->data, m->len,
					   MT_QSEL_EDCA, wiv, &out_len);
	if (ret != RF_2A4M1_S8021X_OK) {
		kfree(frame);
		return -EINVAL;
	}
	/*
	 * Async: this runs in softirq when the core answers a received frame.
	 * On success @frame belongs to the completion handler.
	 */
	ret = rf_2a4m1_bulk_out_async(dev, RF_2A4M1_EP_OUT_DATA, frame, out_len);
	if (ret)
		kfree(frame);
	return ret;
}

static int rf_2a4m1_op_set_key(struct rf_2a4m1_hal *h, u8 slot,
			       const struct rf_2a4m1_key *k)
{
	struct rf_2a4m1_dev *dev = h->priv;

	(void)slot;
	/*
	 * The MT7601U per-WCID HW crypto key table is programmed by
	 * rf_2a4m1_usb_install_hw_keys() at connect completion, from the keys the
	 * in-driver SME derived in its own 4-way (this FullMAC driver runs the
	 * handshake itself, so there is no wpa_supplicant .add_key here).  This op
	 * stays wired for the external-supplicant path (cfg80211 .add_key ->
	 * set_key): a pairwise CCMP key with a peer triggers the same HW install.
	 */
	if (k && k->type == 1 && k->cipher == MT_CIPHER_AES_CCMP &&
	    dev->sme.ptk_installed)
		rf_2a4m1_usb_install_hw_keys(dev);
	else
		dev_dbg(dev->dev, "set_key slot %u (cipher %u) - deferred to connect completion\n",
			slot, k ? k->cipher : 0);
	return 0;
}

static int rf_2a4m1_op_set_sta(struct rf_2a4m1_hal *h, u8 wcid,
			       const struct rf_2a4m1_sta_cfg *s)
{
	struct rf_2a4m1_dev *dev = h->priv;

	if (!dev->hw_inited || !s)
		return -ENODEV;
	/* Program the peer MAC into the WCID address CAM: the RX engine matches a
	 * Protected frame's TA against this to pick the per-STA key slot. */
	rf_2a4m1_reg_write(dev, MT_WCID_ADDR(wcid),
			   get_unaligned_le32(s->addr.a));
	rf_2a4m1_reg_write(dev, MT_WCID_ADDR(wcid) + 4,
			   (u32)s->addr.a[4] | ((u32)s->addr.a[5] << 8));
	return 0;
}

static int rf_2a4m1_op_set_rx_filter(struct rf_2a4m1_hal *h, u32 pass_mask)
{
	struct rf_2a4m1_dev *dev = h->priv;
	u32 cfg = MT_RX_FILTR_CFG_CRC_ERR | MT_RX_FILTR_CFG_PHY_ERR;

	if (!dev->hw_inited)
		return -ENODEV;
	/*
	 * Drop hardware-error frames; accept everything else.  TODO: the
	 * finer per-opmode drop bits (ACK/CTS/RTS/PS-Poll/BA/Ctrl-reserved) are
	 * not yet mapped from the core's pass_mask onto MT_RX_FILTR_CFG.
	 */
	(void)pass_mask;
	rf_2a4m1_reg_write(dev, MT_RX_FILTR_CFG, cfg);
	return 0;
}

static int rf_2a4m1_op_calibrate(struct rf_2a4m1_hal *h,
				 enum rf_2a4m1_hal_cal what)
{
	struct rf_2a4m1_dev *dev = h->priv;

	if (!dev->hw_inited)
		return -ENODEV;
	switch (what) {
	case HAL_CAL_RF:	return rf_2a4m1_mcu_calibrate(dev, MCU_CAL_R, 0);
	case HAL_CAL_RX_DCOC:	rf_2a4m1_rxdc_cal(dev); return 0;
	case HAL_CAL_TX_LO:	return rf_2a4m1_mcu_calibrate(dev, MCU_CAL_LOFT, 0);
	case HAL_CAL_ALL:	return rf_2a4m1_init_cal(dev);
	default:		return 0;	/* HAL_CAL_TEMP: temp_comp not ported */
	}
}

static int rf_2a4m1_op_set_lower_mac(struct rf_2a4m1_hal *h,
				     const struct rf_2a4m1_lmac_cfg *cfg)
{
	struct rf_2a4m1_dev *dev = h->priv;
	u32 slot;

	if (!dev->hw_inited || !cfg)
		return -ENODEV;
	/* Slot time (backoff) + the XIFS/SIFS timing register. */
	slot = rf_2a4m1_reg_read(dev, MT_BKOFF_SLOT_CFG) & ~0xffu;
	rf_2a4m1_reg_write(dev, MT_BKOFF_SLOT_CFG, slot | (cfg->slot_us & 0xff));
	/*
	 * TODO: the per-AC EDCA queue parameters (cw_min/cw_max/aifsn/
	 * txop in cfg->ac[4]) need the MT_EDCA_CFG_AC(n) register block, whose
	 * offsets are not part of this bring-up's ported register set; only the
	 * slot time is wired here.
	 */
	return 0;
}

static const struct rf_2a4m1_hal_ops rf_2a4m1_usb_hal_ops = {
	.start		= rf_2a4m1_op_start,
	.stop		= rf_2a4m1_op_stop,
	.set_channel	= rf_2a4m1_op_set_channel,
	.tx		= rf_2a4m1_op_tx,
	.set_key	= rf_2a4m1_op_set_key,
	.set_sta	= rf_2a4m1_op_set_sta,
	.set_rx_filter	= rf_2a4m1_op_set_rx_filter,
	.calibrate	= rf_2a4m1_op_calibrate,
	.set_lower_mac	= rf_2a4m1_op_set_lower_mac,
};

/* Install the kernel USB HAL ops + the MT7601U capability descriptor into the
 * device's in-module HAL, so the core's hal_* wrappers dispatch to this file. */
void rf_2a4m1_usb_hal_init(struct rf_2a4m1_dev *dev)
{
	struct rf_2a4m1_hal *h = &dev->hal;

	memset(h, 0, sizeof(*h));
	h->ops  = &rf_2a4m1_usb_hal_ops;
	h->priv = dev;

	/* MT7601U: single-chip USB 802.11n 1T1R 2.4 GHz. */
	h->caps.max_gen	  = RF_2A4M1_HAL_GEN_HT;
	h->caps.bands	  = RF_2A4M1_HAL_BAND_2G4;
	h->caps.max_bw_mhz = 40;
	h->caps.tx_chains = 1;
	h->caps.rx_chains = 1;
	h->caps.max_mcs	  = 7;
	h->caps.n_links	  = 1;
	h->caps.lower_mac_does_acks = true;
	h->caps.crypto_offload	    = true;
	h->caps.n_key_slots = 4;
	h->caps.n_sta_slots = 8;
}
