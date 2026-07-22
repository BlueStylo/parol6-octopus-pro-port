# PAROL6 Octopus Pro Port v1.0.0

First public, physically validated release of the PAROL6 low-level controller
firmware port for **BTT Octopus Pro V1.0/V1.0.1 F446**.

## Artifact

- File: `firmware.bin`
- Size: 50,156 bytes
- SHA-256: `66471679470987c078078b68e6dd05ace0703d8d5a25a77b6efaf81afdf7fee8`
- Flash vector address: `0x08008000`
- ELF entry point used to produce this binary: `0x08012499`
- PlatformIO environment: `btt_octopus_pro_v1_0_f446ze_literal_upstream`

## Toolchain

- PlatformIO Core 6.1.19
- Platform `ststm32@19.6.0`
- Arduino Core STM32 2.12.0
- GNU Arm GCC 12.3.1

## Validated behavior

- microSD bootloader update and `FIRMWARE.CUR` rename
- USB CDC enumeration and PAROL6 packet exchange
- six BIGTREETECH TMC5160T drivers and all six robot axes
- final J1–J6 motor/limit mapping
- six end sensors with their final trigger polarities
- STOP7 immediate motion cancellation, holding-torque retention, and explicit
  reset/resume recovery
- original first and repeated full HOME sequence
- Waldo Web Commander motion, HOME, STOP7, and sample dance
- four 12V 4040 cooling fans

FAN0/OUTPUT1 for a 24V single-solenoid valve is implemented and covered by
source/build contracts, but a physical valve-coil and pneumatic switching test
was not formally recorded for this release.

## Compatibility notes

- F446 only; do not flash on Octopus Pro H723 or other MCU variants.
- `MOTOR2` and `MOTOR7` are intentionally unused.
- The validated binary retains the cosmetic USB manufacturer string
  `PAROL6_OCTOPUS_EXP22`.
- Apply the supplied PAROL6 Python API and Waldo patches to preserve the
  documented STOP7 torque-hold and explicit-recovery behavior.

## Flashing

Verify the SHA-256 digest, copy `firmware.bin` to the root of a FAT32 microSD
card, insert it while power is off, then power the board. Read the repository's
build/flash guide and safety warning before connecting motor power.

This is an unofficial, non-certified experimental robot controller port.
