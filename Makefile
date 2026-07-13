# SPDX-License-Identifier: GPL-2.0-only
#
# Out-of-tree kbuild entry point for rf-2a4m1.ko.
# The object list + compile flags live in Kbuild. DKMS invokes the same
# `make -C <kdir> M=<src> modules` path (see dkms.conf).

KVER  ?= $(shell uname -r)
KDIR  ?= /lib/modules/$(KVER)/build
PWD   := $(shell pwd)

# Single source-of-truth version, shared with dkms.conf / packaging.
VERSION := $(shell cat $(PWD)/VERSION)

.PHONY: default modules clean version help

default: modules

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

# Propagate the VERSION file into dkms.conf's PACKAGE_VERSION and the driver's
# MODULE_VERSION header. (Regenerator added with the driver source.)
version:
	@echo "VERSION = $(VERSION)"

help:
	@echo "make            - build rf-2a4m1.ko against $(KDIR)"
	@echo "make clean      - remove build outputs"
	@echo "make version    - show the source-of-truth version ($(VERSION))"
