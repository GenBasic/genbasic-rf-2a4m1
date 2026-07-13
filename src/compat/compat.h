/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Cross-kernel compatibility shims for rf-2a4m1.
 *
 * Force-included into every translation unit (see Kbuild). Each shim is gated
 * on LINUX_VERSION_CODE so one source tree builds against every supported
 * stable kernel. Populated alongside the driver source; the supported floor is
 * enforced by BUILD_EXCLUSIVE_KERNEL in dkms.conf.
 */
#ifndef RF_2A4M1_COMPAT_H
#define RF_2A4M1_COMPAT_H

#include <linux/version.h>

/* cfg80211 / mac80211 / USB API shims are added here as the driver source
 * lands, e.g.:
 *
 *   #if LINUX_VERSION_CODE < KERNEL_VERSION(x, y, z)
 *   ... backport ...
 *   #endif
 */

#endif /* RF_2A4M1_COMPAT_H */
