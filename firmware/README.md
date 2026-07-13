<!-- SPDX-License-Identifier: LicenseRef-Proprietary -->

# `firmware/` — the on-chip firmware image

`rf-2a4m1.bin` is the firmware image the RF-2A4M1 adapter runs. It is
proprietary (see [../LICENSE.firmware](../LICENSE.firmware)), loaded at runtime
by the driver, and never linked into the kernel module.

| Path | Contents |
|------|----------|
| `rf-2a4m1.bin` | the on-chip firmware image (shipped in `rf-2a4m1-firmware`) |
| `Makefile` | builds the image + packs the container |
| `mkfw` | packs the built image into the on-disk container |

Install path: `/lib/firmware/rf-2a4m1/rf-2a4m1.bin`. The driver advertises the
dependency via `MODULE_FIRMWARE("rf-2a4m1/rf-2a4m1.bin")`.

The image and its build are added in a subsequent commit.
