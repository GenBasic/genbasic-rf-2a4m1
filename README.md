<!-- SPDX-License-Identifier: GPL-2.0-only -->

# RF-2A4M1 — GenBasic 2.4 GHz USB Wi-Fi driver

`rf-2a4m1` is the Linux driver for the **GenBasic RF-2A4M1**, a single-band
2.4 GHz (1T1R) 802.11b/g/n USB Wi-Fi adapter.

It follows the standard open-driver + on-device-firmware model — an open,
source-built kernel module plus a firmware image the device runs:

- **`rf-2a4m1-dkms`** — a **GPL** `cfg80211` kernel module. DKMS builds it from
  source against whatever stable kernel is installed, so the package is
  architecture-independent and rebuilds automatically on every kernel upgrade.
- **`rf-2a4m1-firmware`** — the firmware image `rf-2a4m1.bin` that the adapter
  runs. It is loaded at runtime and is never linked into the kernel module.

## Status

Early scaffold: the repository layout, build files, and packaging are in place;
the driver source (`src/`) and the firmware image (`firmware/rf-2a4m1.bin`) are
added in subsequent commits.

## Building from source (DKMS)

```sh
# Debian/Ubuntu
sudo apt install ./rf-2a4m1-dkms_<ver>_all.deb ./rf-2a4m1-firmware_<ver>_all.deb

# RHEL/Alma/Rocky/Fedora
sudo dnf install ./rf-2a4m1-<ver>-1.noarch.rpm ./rf-2a4m1-firmware-<ver>-1.noarch.rpm
```

DKMS compiles `rf-2a4m1.ko` against the running kernel and every installed
kernel. On a SecureBoot machine, DKMS signs the module with the machine-owner's
MOK; complete the one-time `mokutil --import` enrollment the post-install step
prints.

To build directly against the running kernel for development:

```sh
make            # builds rf-2a4m1.ko out-of-tree
sudo insmod rf-2a4m1.ko
```

## Driver binding

The adapter shares a USB device-ID range with an in-kernel driver, so the
firmware package ships `/etc/modprobe.d/rf-2a4m1.conf` to ensure `rf_2a4m1`
claims the device. On a host that must also run other devices in that ID range,
use the optional `driver_override` rule instead of the global blacklist — see
`conf/`.

## Layout

| Path | Contents |
|------|----------|
| `src/` | the GPL `cfg80211` driver source |
| `firmware/` | the firmware image `rf-2a4m1.bin` + its build |
| `packaging/` | `rf-2a4m1-dkms` + `rf-2a4m1-firmware` `.deb` / `.rpm` sources |
| `conf/` | modprobe / modules-load / udev integration |
| `ci/` | the build matrix |

## License

The driver (`src/`, build scaffold, packaging) is **GPL-2.0-only** — see
[`LICENSE`](LICENSE). The firmware image `rf-2a4m1.bin` is proprietary — see
[`LICENSE.firmware`](LICENSE.firmware).

Copyright © GenBasic. `MODULE_AUTHOR("GenBasic")`.
