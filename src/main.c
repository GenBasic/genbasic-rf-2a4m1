// SPDX-License-Identifier: GPL-2.0-only
/*
 * RF-2A4M1 - GPL Wi-Fi driver for MediaTek MT7601U USB silicon.
 *
 * Module init/exit, USB device table, per-device lifecycle, the firmware-load
 * trigger, and the platform seam the core needs (memcpy/memset
 * are the kernel's; entropy/time/critical-section hooks are mapped here).
 *
 * Copyright (C) GenBasic.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/slab.h>
#include <linux/firmware.h>
#include <linux/random.h>
#include <linux/etherdevice.h>
#include <linux/ktime.h>
#include <linux/math64.h>
#include <linux/spinlock.h>

#include "rf-2a4m1.h"

/* ================================================================== */
/* Platform seam consumed by the core (declared in the core   */
/* header).  memcpy/memset resolve to the kernel's own; these three     */
/* hooks map onto kernel facilities.                                    */
/* ================================================================== */
size_t plat_random(uint8_t *buf, size_t len)
{
	get_random_bytes(buf, len);
	return len;
}

uint64_t plat_time_ms(void)
{
	return div_u64(ktime_get_ns(), NSEC_PER_MSEC);
}

/*
 * The core is run-to-completion and the glue serializes every core entry under
 * this critical section.  It is a module-global spinlock: correct (safe) for
 * any number of bound dongles, but broader than necessary - it serializes
 * across devices rather than per device.
 *
 * TODO: a true per-rf_2a4m1_dev lock is blocked at the core API - the
 * platform seam is plat_crit_enter()/plat_crit_exit() with NO device handle,
 * so the glue cannot select a per-device lock from inside them.  Making it
 * per-device needs the core to thread a device/context token through the
 * critical-section seam (a core-side change), not a glue change.
 */
static DEFINE_SPINLOCK(rf_2a4m1_crit_lock);
static unsigned long rf_2a4m1_crit_flags;

void plat_crit_enter(void)
{
	spin_lock_irqsave(&rf_2a4m1_crit_lock, rf_2a4m1_crit_flags);
}

void plat_crit_exit(void)
{
	spin_unlock_irqrestore(&rf_2a4m1_crit_lock, rf_2a4m1_crit_flags);
}

/* ================================================================== */
/* USB device table.                                                   */
/* ================================================================== */
static const struct usb_device_id rf_2a4m1_id_table[] = {
	{ USB_DEVICE(0x0e8d, 0x7601) },	/* MediaTek MT7601U */
	{ USB_DEVICE(0x148f, 0x7601) },	/* Ralink/MediaTek MT7601U */
	{ USB_DEVICE(0x148f, 0x760b) },	/* MT7601U (OEM) */
	{ }
};
MODULE_DEVICE_TABLE(usb, rf_2a4m1_id_table);

/* ================================================================== */
/* Probe / disconnect.                                                 */
/* ================================================================== */
static int rf_2a4m1_probe(struct usb_interface *intf,
			  const struct usb_device_id *id)
{
	struct rf_2a4m1_dev *dev;
	const struct firmware *fw;
	int ret;

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	dev->udev = usb_get_dev(interface_to_usbdev(intf));
	dev->intf = intf;
	dev->dev  = &intf->dev;
	dev->state = RF_2A4M1_STATE_INIT;
	mutex_init(&dev->lock);
	usb_set_intfdata(intf, dev);

	/* Provisional MAC: overwritten by the real chip-EEPROM MAC once the chip
	 * bring-up runs below; a locally-administered random MAC bridges the gap
	 * if the firmware/EEPROM path is unavailable. */
	eth_random_addr(dev->macaddr.a);

	ret = rf_2a4m1_usb_probe_setup(dev);
	if (ret)
		goto err;

	rf_2a4m1_usb_hal_init(dev);

	/* Firmware-load trigger, then the MAC/BBP/RF chip bring-up (which reads
	 * the real MAC from the chip EEPROM into dev->macaddr).  Both must happen
	 * before cfg80211_register so the net_device gets the real hardware MAC. */
	ret = request_firmware(&fw, RF_2A4M1_FIRMWARE, dev->dev);
	if (ret == 0) {
		ret = rf_2a4m1_usb_load_firmware(dev, fw->data, fw->size);
		release_firmware(fw);
		if (ret) {
			dev_warn(dev->dev, "firmware load failed (%d); continuing without MCU\n",
				 ret);
		} else {
			ret = rf_2a4m1_chip_init(dev, 1 /* default channel */);
			if (ret)
				dev_warn(dev->dev, "chip init failed (%d); continuing (RX/TX inactive)\n",
					 ret);
		}
	} else {
		dev_warn(dev->dev, "firmware '%s' not found (%d); continuing (chip MCU not started)\n",
			 RF_2A4M1_FIRMWARE, ret);
	}

	dev->wiphy = rf_2a4m1_wiphy_alloc(dev->dev);
	if (!dev->wiphy) {
		ret = -ENOMEM;
		goto err;
	}

	ret = rf_2a4m1_cfg80211_register(dev);
	if (ret)
		goto err_wiphy;

	dev->state = RF_2A4M1_STATE_RUNNING;
	dev_info(dev->dev, "rf-2a4m1 bound (%04x:%04x)\n",
		 id->idVendor, id->idProduct);
	return 0;

err_wiphy:
	wiphy_free(dev->wiphy);
	dev->wiphy = NULL;
err:
	usb_set_intfdata(intf, NULL);
	usb_put_dev(dev->udev);
	kfree(dev);
	return ret;
}

static void rf_2a4m1_disconnect(struct usb_interface *intf)
{
	struct rf_2a4m1_dev *dev = usb_get_intfdata(intf);

	if (!dev)
		return;

	dev->state = RF_2A4M1_STATE_REMOVING;
	rf_2a4m1_usb_rx_stop(dev);
	rf_2a4m1_cfg80211_unregister(dev);
	if (dev->wiphy)
		wiphy_free(dev->wiphy);
	usb_set_intfdata(intf, NULL);
	usb_put_dev(dev->udev);
	kfree(dev);
	dev_info(&intf->dev, "rf-2a4m1 unbound\n");
}

static struct usb_driver rf_2a4m1_driver = {
	.name		= "rf-2a4m1",
	.id_table	= rf_2a4m1_id_table,
	.probe		= rf_2a4m1_probe,
	.disconnect	= rf_2a4m1_disconnect,
	.soft_unbind	= 1,
};

module_usb_driver(rf_2a4m1_driver);

MODULE_DESCRIPTION("GenBasic RF-2A4M1 2.4 GHz USB Wi-Fi (MediaTek MT7601U)");
MODULE_AUTHOR("GenBasic");
MODULE_LICENSE("GPL");
MODULE_VERSION(RF_2A4M1_VERSION);
MODULE_FIRMWARE(RF_2A4M1_FIRMWARE);
