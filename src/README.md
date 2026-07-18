<!-- SPDX-License-Identifier: GPL-2.0-only -->

# `src/` — the GPL cfg80211 driver

All of `src/` is GPL-2.0-only, compiled from source per-kernel by DKMS.

| Path | Contents |
|------|----------|
| `rf-2a4m1.h` | driver private header (state, dev struct, firmware macros) |
| `usb.c` | USB transport — endpoints, bulk/control transfers, probe/disconnect |
| `cfg80211.c` | the `cfg80211` / `mac80211` interface |
| `main.c` | module init/exit, device lifecycle, firmware load |
| `core/` | the driver core (MAC / MLME / crypto) |
| `compat/compat.h` | `LINUX_VERSION_CODE`-gated shims for cross-kernel builds |

`Kbuild` (repo root) and `core/objects.mk` list the object set and compile
flags; DKMS builds every `.c` here against each installed kernel.
