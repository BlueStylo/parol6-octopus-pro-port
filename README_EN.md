# PAROL6 Octopus Pro Port

Unofficial experimental PAROL6 firmware port and bring-up notes for the BTT
Octopus Pro V1.0 F446 board.

## Status

This repository is a private work log for an in-progress hardware and firmware
experiment. It is not a finished controller replacement, not a drop-in
alternative to the official PAROL6 control board, and not affiliated with
Source Robotics.

Current focus:

- Adapting the GPLv3 PAROL6 low-level control-board firmware to an STM32F446ZE
  based BTT Octopus Pro V1.0 board.
- Preserving the PAROL6 protocol and joint ordering while remapping step, dir,
  chip-select, enable, limit, and E-stop pins.
- Documenting safe bring-up steps for TMC5160 drivers, 24 V sensors, dry-contact
  limit switches, USB CDC, and bootloader offset.

## Upstream

Original PAROL6 project:

- Repository: https://github.com/Source-Robotics/PAROL6-Desktop-robot-arm
- Local upstream snapshot used during this work:
  `896fa9fd793a076a0b3790b8a4a29a6f5bb12fcb`
- License: GPLv3

The upstream source snapshot is intentionally not vendored in this repository.
Clone the original repository separately when comparing against upstream.

## Repository Layout

- `01_Research/` - research notes and hardware fit assessment
- `03_Board_Firmware/` - Octopus Pro firmware port work
- `06_Docs/` - wiring, pin mapping, and bring-up notes
- `000_INDEX.md` - local workspace index

## Firmware Port

The current firmware draft lives at:

`03_Board_Firmware/01_20260606_OctopusProV1_Port__InProgress`

Key assumptions:

- Board: BTT Octopus Pro V1.0 F446 / STM32F446ZE
- Framework: PlatformIO with Arduino STM32 core
- Bootloader offset: 32 KiB / `0x08008000`
- Clock: 12 MHz HSE
- Motor sockets: PAROL6 J1-J6 mapped onto Octopus MOTOR0-MOTOR5
- Driver control: TMC5160-style SPI control using `TMCStepper`
- USB serial: CDC, matching the PAROL6 firmware baud rate expectation

See `06_Docs/01_20260606_OctopusProV1_Port_Notes.md` for the current pin map
and bring-up order.

## Safety Notes

This project involves moving robotic hardware, motor drivers, 24 V wiring, and
firmware-controlled motion. Do not assume the draft firmware is safe on real
hardware.

Important defaults:

- Do not connect PAROL6 24 V inductive sensors directly to Octopus endstop pins.
- Use an appropriate 24 V to 3.3 V interface, such as an optocoupler module, and
  verify polarity before homing.
- Treat firmware E-stop reporting as telemetry only. A real E-stop should remove
  motor power or disable drives through a hardware safety path independent of
  MCU code.
- Test one subsystem at a time with motor power disconnected whenever possible.

Read `SAFETY_WARNING_AND_DISCLAIMER.md` before using any hardware notes or
firmware from this repository.

## License

This repository contains derivative firmware work based on GPLv3 PAROL6 source
code, so the repository is distributed under GPLv3. Third-party libraries retain
their own license notices in their respective directories.
