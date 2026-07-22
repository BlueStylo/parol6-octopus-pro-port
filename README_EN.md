# PAROL6 Octopus Pro Port

[한국어](README.md) · [Safety warning](SAFETY_WARNING_AND_DISCLAIMER.md) · [Wiring map](06_Docs/02_20260722_Final_Wiring_and_Pin_Map.md) · [Firmware binary](07_Output/01_20260722_PAROL6_OctopusPro_v1.0.0__Final/firmware.bin)

An unofficial port of the original PAROL6 low-level controller firmware to the **BTT Octopus Pro V1.0/V1.0.1 F446**. It retains the PAROL6 56/60-byte wire protocol, joint profiles, and upstream homing state machine as literally as possible while adapting board pins, drivers, USB, STOP7, and auxiliary outputs.

> This is not an official Source Robotics or BIGTREETECH product, and it is not a certified safety controller. Read the [safety warning](SAFETY_WARNING_AND_DISCLAIMER.md) before working with moving machinery or 24V power.

## v1.0.0 status

- Validated hardware: **BTT Octopus Pro V1.0.1 F446 / STM32F446ZET6**
- Drivers: **BIGTREETECH TMC5160T**, SPI, 32 microsteps
- Robot and motor supply used during validation: 24V
- Physically validated: SD flashing, USB CDC, all six axes, all six limit sensors, STOP7 halt/torque hold/recovery, first and repeated full HOME, Waldo Web Commander, sample dance, and four cooling fans
- Implemented and statically validated: FAN0 24V single-solenoid output
- Not formally recorded as physically validated: the pneumatic valve coil and pneumatic switching

Release artifacts:

- [Final firmware source](03_Board_Firmware/02_20260719_OctopusPro_PAROL6_Exp23__Final/)
- [v1.0.0 firmware and release notes](07_Output/01_20260722_PAROL6_OctopusPro_v1.0.0__Final/)
- [Waldo/PAROL6 Python API compatibility patches](02_Host_Integration/01_20260718_Waldo_STOP7_Torque_Hold_Patches__Final/)
- [Octopus Pro controller enclosure STL files](04_3D_Printing/01_Octopus_Pro_Board_Enclosure/)

## Supported target

This release targets the **Octopus Pro F446 variants only**. Do not flash it on H723 or other MCU variants.

| Item | Configuration |
|---|---|
| MCU | STM32F446ZET6, 180MHz |
| External clock | 12MHz HSE |
| Bootloader offset | 32KiB, `0x08008000` |
| USB | CDC, VID:PID `0483:5740`, nominal 3,000,000 baud |
| Motor supply | Physically validated at 24V |
| SPI | SCK `PA5`, MISO `PA6`, MOSI `PA7` |

The compiled USB manufacturer string remains `PAROL6_OCTOPUS_EXP22` to preserve the exact validated v1.0.0 binary. It is cosmetic and does not affect operation.

## Final joint and sensor map

| Joint | Motor socket | STEP | DIR | CS | EN | Limit | Pin | Trigger |
|---|---|---|---|---|---|---|---|---|
| J1 | MOTOR6 | `PE2` | `PE3` | `PE1` | `PD4` | STOP3 | `PG11` | LOW |
| J2 | MOTOR5 | `PC13` | `PF0` | `PE4` | `PF1` | STOP2 | `PG10` | HIGH |
| J3 | MOTOR4 | `PF9` | `PF10` | `PF2` | `PG2` | STOP1 | `PG9` | HIGH |
| J4 | MOTOR3 | `PG4` | `PC1` | `PC7` | `PA0` | STOP4 | `PG12` | LOW |
| J5 | MOTOR1 | `PG0` | `PG1` | `PD11` | `PF15` | STOP0 | `PG6` | HIGH |
| J6 | MOTOR0 | `PF13` | `PF12` | `PC4` | `PF14` | STOP5 | `PG13` | LOW |

`MOTOR2` is intentionally bypassed because the Octopus board exposes it as the parallel Z output arrangement; `MOTOR7` is also unused. See the [final wiring map](06_Docs/02_20260722_Final_Wiring_and_Pin_Map.md) for sensor types, jumpers, fans, and valve wiring.

## STOP7 behavior

STOP7 uses a **normally closed** contact on `PG15`.

- Healthy closed circuit: LOW
- Button pressed or wire broken: HIGH
- HIGH immediately gates STEP low and cancels active motion/HOME.
- Already enabled drivers retain EN, preserving holding torque on gravity-loaded axes.
- Releasing the button never resumes automatically. Use Reset and then a separate Resume/Enable action through the patched host.
- If STOP7 is open at boot, drivers are never enabled and holding torque is unavailable.

This is a **software halt with torque hold**, designed for a robot that can fall when motor power is removed. It is not a safety-rated hard E-stop. Keep a separate, immediately accessible 24V power cutoff.

## Quick start

1. Read the [build, flash, and first-start guide](06_Docs/03_20260722_Build_Flash_and_First_Start.md) and the safety warning.
2. With all power off, verify that the board is an Octopus Pro **F446**, then verify supply polarity, driver voltage-selection jumpers, and SPI jumpers.
3. Never connect a 24V proximity sensor directly to an MCU input. Isolate or level-shift it to 3.3V logic.
4. Copy [`firmware.bin`](07_Output/01_20260722_PAROL6_OctopusPro_v1.0.0__Final/firmware.bin) to the root of a FAT32 microSD card.
5. Insert the card while powered off, then power the board. A successful flash renames the file to `FIRMWARE.CUR`.
6. Verify STOP7 and all limit inputs before validating one axis at a time under low-risk conditions.
7. When using Waldo, apply the [host compatibility patches](02_Host_Integration/01_20260718_Waldo_STOP7_Torque_Hold_Patches__Final/) so STOP7 torque-hold and manual-recovery semantics are preserved.

Release binary SHA-256:

```text
66471679470987c078078b68e6dd05ace0703d8d5a25a77b6efaf81afdf7fee8  firmware.bin
```

## Building from source

The validated toolchain used PlatformIO Core 6.1.19 with `ststm32@19.6.0`.

```bash
cd 03_Board_Firmware/02_20260719_OctopusPro_PAROL6_Exp23__Final
pio run
```

The default environment is `btt_octopus_pro_v1_0_f446ze_literal_upstream`. See the [firmware README](03_Board_Firmware/02_20260719_OctopusPro_PAROL6_Exp23__Final/README.md) for build and test details.

## Auxiliary outputs

- FAN4/`PD14` and FAN5/`PD15`: ON after boot
- FAN6 and FAN7: hard-wired to `V_FAN`, always on
- Validated installation: four 12V 4040 fans; the fan voltage rail must be set to 12V
- FAN0/`PA8` = PAROL6 `OUTPUT1`: 24V single-solenoid output, LOW=OFF/closed and HIGH=ON/open

For a 24V FAN0 load, place that port on `VIN/V_FUSED` and verify coil rating, polarity, and suppression before wiring. The release verifies the protocol, pin contract, and boot-off state; it does not include a recorded physical pneumatic-valve test.

## Repository layout

- `02_Host_Integration/` — host patches preserving STOP7 recovery semantics
- `03_Board_Firmware/02_...__Final/` — reproducible final firmware source
- `04_3D_Printing/` — enclosure STL files for Octopus Pro and four 4040 fans
- `06_Docs/` — final wiring, flashing, and validation documents
- `07_Output/01_...v1.0.0__Final/` — release binary, checksum, and release notes

The firmware directory numbered `01` and the 2026-06-06 notes are superseded development drafts and must not be used for final wiring.

## Upstream and license

This port is derived from the GPLv3 firmware in [PCrnjak/PAROL6-Desktop-robot-arm](https://github.com/PCrnjak/PAROL6-Desktop-robot-arm), pinned at commit `896fa9fd793a076a0b3790b8a4a29a6f5bb12fcb`. Automated tests verify that the original motion protocol and HOME functions remain literal upstream code.

Derivative firmware and project material are distributed under the root [GPLv3 LICENSE](LICENSE). Third-party copyright and license terms are listed in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) and in each bundled library's LICENSE file.

## Validation and known limitations

See the [hardware validation report](06_Docs/04_20260722_Hardware_Validation_Report.md) for the exact tested scope.

- This is a port validated on one physical build, not a general-purpose finished controller.
- STOP7, USB communications, sensors, and host applications are not safety certified.
- FAN0 has no watchdog and may retain its last output state across a USB disconnect or STOP7 event.
- Full program execution in Better Command is not claimed as validated by this release.
- Always verify mechanical assembly, sensor position, and travel clearance before automatic HOME.
