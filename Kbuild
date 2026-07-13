# SPDX-License-Identifier: GPL-2.0-only
#
# Object list for rf-2a4m1.ko (runtime module name rf_2a4m1).

obj-m += rf-2a4m1.o

# USB transport + cfg80211 interface + init/lifecycle.
rf-2a4m1-y := src/usb.o src/cfg80211.o src/main.o

# The driver core objects (added by a subsequent commit into src/core/objects.mk).
-include $(src)/src/core/objects.mk

ccflags-y += -I$(src)/src -I$(src)/src/core -I$(src)/src/compat
ccflags-y += -include $(src)/src/compat/compat.h
