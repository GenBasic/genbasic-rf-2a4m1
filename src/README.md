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

The `.c` sources and `core/` are added in a subsequent commit; the `Kbuild`
object list and compile flags are already in place at the repo root.
