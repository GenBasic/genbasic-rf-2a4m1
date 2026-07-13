<!-- SPDX-License-Identifier: GPL-2.0-only -->

# `ci/` — the build matrix

The driver is architecture-independent GPL source compiled per-kernel by DKMS,
so CI proves it builds across the supported kernel range:

- **Build:** compile the module against each supported kernel's headers (Debian,
  Ubuntu LTS, RHEL / Alma / Rocky, Fedora, mainline LTS + latest stable); fail on
  any build error.
- **Load-test:** on the shipped kernels, load the module, bind the device, load
  the firmware, and bring up the interface.

Runs on a local runner. The workflow definition is added with the driver source.
