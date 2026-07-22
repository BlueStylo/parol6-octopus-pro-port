# Third-Party Notices

This repository contains or builds upon third-party open-source software. The
notices below are informational; the license text shipped with each component
controls.

## PAROL6 control-board firmware

- Project: PAROL6 Desktop Robot Arm
- Upstream: <https://github.com/PCrnjak/PAROL6-Desktop-robot-arm>
- Author/project owner: Petar Crnjak / Source Robotics
- Pinned commit: `896fa9fd793a076a0b3790b8a4a29a6f5bb12fcb`
- Firmware tree: `2bac41ef589ae8e253384a977c83b8526726becc`
- License: GNU General Public License version 3

The Octopus Pro firmware is a modified version of this firmware. The repository
root `LICENSE`, the final firmware source, and `UPSTREAM.lock` are provided with
the release.

## AccelStepper 1.64

- Project: AccelStepper
- Authors: Mike McCauley and contributors
- Version: 1.64
- Upstream: <https://www.airspayce.com/mikem/arduino/AccelStepper/>
- License: GNU General Public License version 3
- Bundled notice: `03_Board_Firmware/02_20260719_OctopusPro_PAROL6_Exp23__Final/lib/AccelStepper-master/LICENSE`

## TMCStepper 0.7.3

- Project: TMCStepper
- Author: teemuatlut and contributors
- Version: 0.7.3
- Upstream: <https://github.com/teemuatlut/TMCStepper>
- License: MIT
- Bundled notice: `03_Board_Firmware/02_20260719_OctopusPro_PAROL6_Exp23__Final/lib/TMCStepper-master/LICENSE`

## Arduino Core for STM32 2.12.0

- Project: STM32duino Arduino Core for STM32
- PlatformIO package: `framework-arduinoststm32` 4.21200.0
- Arduino core version: 2.12.0
- Upstream: <https://github.com/stm32duino/Arduino_Core_STM32>
- Licenses: component-specific, including LGPL-2.1, BSD-3-Clause, Apache-2.0,
  MIT, and the STM32 USB library license described by the upstream project

The framework is not vendored in this repository. PlatformIO obtains the pinned
framework through `ststm32@19.6.0`; its complete source and per-file license
headers are available from the upstream repository and the PlatformIO package.

## CMSIS, STM32 HAL, and GNU Arm toolchain runtime

The generated firmware also contains components selected by the STM32 Arduino
framework, including CMSIS, STM32 HAL, USB device code, and compiler runtime
support. Their individual source headers and license terms are supplied by the
corresponding STM32duino and GNU Arm toolchain packages.

## Host-side projects referenced by patches

The host compatibility directory contains patches, not complete copies, for:

- PAROL6 Python API v0.3.1: <https://github.com/PCrnjak/PAROL6-python-API>,
  pinned commit `8f6b3d19ddd3fb854af9890d89c7c96ca093cb7c`
- Waldo Commander: <https://github.com/Jepson2k/Waldo-Commander>, pinned commit
  `72d7ced4da1a8b0ababd4ca9a39a0693523b098f`

Users must retain the license and copyright notices from those upstream
projects when applying or redistributing the patches.
