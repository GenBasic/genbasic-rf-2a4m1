/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * RF-2A4M1 (GenBasic) 2.4 GHz USB Wi-Fi driver — private header.
 * Copyright (C) GenBasic.
 */
#ifndef RF_2A4M1_H
#define RF_2A4M1_H

#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>

#define RF_2A4M1_VERSION        "1.0.0"

/* Firmware image, loaded at runtime from /lib/firmware/. */
#define RF_2A4M1_FIRMWARE       "rf-2a4m1/rf-2a4m1.bin"

/* Firmware-image compatibility gate (asserted after the container header parse,
 * before upload). The family tag distinguishes an RF-2A4M1 image from any other
 * image sharing the same on-disk container; the min-version floor prevents a
 * newer driver from driving an incompatible older image. */
#define RF_2A4M1_FW_FAMILY_TAG  0x2a41
#define RF_2A4M1_FW_MIN_VER     0x1000

/* Device lifecycle. */
enum rf_2a4m1_state {
	RF_2A4M1_STATE_INIT = 0,
	RF_2A4M1_STATE_FW_LOADED,
	RF_2A4M1_STATE_RUNNING,
	RF_2A4M1_STATE_REMOVING,
};

struct rf_2a4m1_dev {
	struct usb_device	*udev;
	struct ieee80211_hw	*hw;		/* SoftMAC: MAC in the host driver */
	struct device		*dev;
	enum rf_2a4m1_state	state;
	struct mutex		lock;
	struct workqueue_struct	*wq;
	/* USB HAL, host-core, and cfg80211 members are added by the modules that
	 * own them (src/usb.c, src/core/, src/cfg80211.c). */
};

#endif /* RF_2A4M1_H */
