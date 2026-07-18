# SPDX-License-Identifier: GPL-2.0-only
#
# Object list for the driver core (MAC / MLME / crypto), appended to
# rf-2a4m1-y. Paths are relative to the module root ($(src)).

rf-2a4m1-core-y := \
	src/core/rf_2a4m1_core.o \
	src/core/rf_2a4m1_crypto.o \
	src/core/rf_2a4m1_ht.o \
	src/core/rf_2a4m1_pmf.o \
	src/core/rf_2a4m1_qos.o \
	src/core/rf_2a4m1_roam.o \
	src/core/rf_2a4m1_sme.o \
	src/core/rf_2a4m1_tdls.o \
	src/core/rf_2a4m1_usb_hal.o

rf-2a4m1-y += $(rf-2a4m1-core-y)
