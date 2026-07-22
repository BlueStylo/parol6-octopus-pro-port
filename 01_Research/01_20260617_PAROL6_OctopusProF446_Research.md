# PAROL6 + BTT Octopus Pro F446 Research Notes

> **SUPERSEDED BASELINE — DO NOT USE FOR FINAL WIRING.**
>
> This document records the initial research stage and contains the abandoned
> MOTOR0–MOTOR5 draft mapping. Use the
> [validated final wiring map](../06_Docs/02_20260722_Final_Wiring_and_Pin_Map.md)
> for the completed robot.

Date: 2026-06-17

## Purpose

This note records the research baseline before continuing PAROL6 work with a
BIGTREETECH Octopus Pro using an STM32F446 MCU instead of the original PAROL6
control board.

The main conclusion is that the Octopus Pro F446 is a plausible motion-control
platform for PAROL6, but it is not a drop-in replacement for the PAROL6 control
board. The MCU, step/dir outputs, SPI driver control, and USB CDC path are a
good fit. The safety, 24 V sensor interface, isolated I/O, power latch, and
bring-up workflow need explicit design and verification.

## Confirmed Source Facts

### PAROL6 original electronics

- PAROL6 is a 6-axis desktop robot arm with open-source control software, GUI,
  and STL files.
- The official docs describe the original PAROL6 control board as an
  STM32F446RE-based controller.
- Original board hardware specs include:
  - STM32F446RE, Cortex-M4, 180 MHz, 512 KB flash.
  - TMC5160 stepper drivers over SPI.
  - 10-35 V stepper driver operating range.
  - 24 V robot supply, with operating limit listed as 18-24 V.
  - 2 isolated inputs, 2 isolated outputs, E-stop input, USB, CAN, external SPI
    flash, JTAG/programming adapter, and 5 V fan connection.
- Original docs state that limit switches and inductive sensors are wired as
  24 V signals on the PAROL6 control board:
  - Limit switch: one lead to 24 V and the other to signal.
  - Inductive sensor: brown = 24 V, blue = GND, black = signal.
- Official getting-started docs recommend using the dedicated test firmware to
  verify individual motors, drivers, limit switches, E-stop, and I/O before
  normal operation.
- The original low-level firmware is the PAROL6 control board software. The
  high-level commander/Python/ROS side expects the low-level board to preserve
  the serial protocol and robot state behavior.

### Original firmware shape in this workspace

Local source:

- `02_Source/01_PAROL6_Desktop_Robot_Arm__Upstream/PAROL6 control board main software`

Important local findings:

- `platformio.ini` targets `genericSTM32F446RE` using the Arduino STM32 core.
- USB serial is exposed as `SerialUSB` at 3,000,000 baud.
- The original code uses TMC5160 drivers through `TMCStepper`.
- The original code uses `AccelStepper` with step/dir pins.
- `constants.h` sets `R_SENSE` to `0.075f`, `MICROSTEP` to `32`, and motor
  current limits from 965 mA to 2000 mA.
- The internal PUL/DIR/SELECT numbering is not the same as joint numbering:

| Joint | Original firmware step/dir/select group |
|---|---|
| J1 | PUL1 / DIR1 / SELECT1 |
| J2 | PUL6 / DIR6 / SELECT6 |
| J3 | PUL5 / DIR5 / SELECT5 |
| J4 | PUL4 / DIR4 / SELECT4 |
| J5 | PUL2 / DIR2 / SELECT2 |
| J6 | PUL3 / DIR3 / SELECT3 |

This non-linear mapping is intentional and must be preserved unless we also
change all firmware assumptions around joint order.

### BTT Octopus Pro F446 facts

- BIGTREETECH describes Octopus Pro V1.0 as a 3D-printer motherboard with up to
  8 60 V stepper driver slots and 9 stepper outputs.
- The F446 variant uses an STM32F446ZET6, Cortex-M4, 180 MHz, 512 KB flash.
- Octopus Pro supports Marlin, Klipper, and RepRapFirmware variants.
- BTT warns that each driver slot has a voltage-selection jumper. The driver can
  be fed from the dedicated motor-power input, up to 60 V, or from main board
  power, normally 24 V. The wrong jumper position can destroy non-high-voltage
  drivers and possibly the motherboard.
- BTT also warns that Octopus Pro V1.1 changes several pins relative to
  V1.0/V1.0.1:
  - HE0: PA2 -> PA0
  - HE2: PB10 -> PB0
  - Motor4-EN: PA0 -> PA2
  - RGB: PB0 -> PB10
- Klipper's official Octopus Pro V1.0 config says STM32F446 boards should be
  compiled with a 32 KiB bootloader and a 12 MHz crystal clock reference.
- Klipper's Octopus Pro V1.0 pin config confirms the first six driver slots:

| Socket | STEP | DIR | EN | Driver comm/CS/UART pin | Endstop/DIAG |
|---|---|---|---|---|---|
| MOTOR0 | PF13 | PF12 | PF14, active low | PC4 | PG6 |
| MOTOR1 | PG0 | PG1 | PF15, active low | PD11 | PG9 |
| MOTOR2 | PF11 | PG3 | PG5, active low | PC6 | PG10 |
| MOTOR3 | PG4 | PC1 | PA0, active low | PC7 | PG11 |
| MOTOR4 | PF9 | PF10 | PG2, active low | PF2 | PG12 |
| MOTOR5 | PC13 | PF0 | PF1, active low | PE4 | PG13 |
| MOTOR6 | PE2 | PE3 | PD4, active low | PE1 | PG14 |
| MOTOR7 | PE6 | PA14 | PE0, active low | PD3 | PG15 |

## Current Local Octopus Port Status

Local working copy:

- `03_Board_Firmware/01_20260606_OctopusProV1_Port__InProgress`

Existing local notes:

- `06_Docs/01_20260606_OctopusProV1_Port_Notes.md`

Existing draft build output:

- `07_Output/01_20260606_OctopusProV1_Firmware__Draft/parol6_octopus_pro_v1_f446_bootloader32k_draft.bin`

Current port assumptions:

- Board JSON uses `stm32f446zet6`.
- Flash offset is `0x8000`; upload offset is `0x08008000`, preserving a 32 KiB
  BTT bootloader.
- `HSE_VALUE=12000000U` is defined for the Octopus Pro F446 12 MHz crystal.
- SPI remains on PA5/PA6/PA7, matching the original firmware's SPI1 pins.
- The port replaces the original single `GLOBAL_ENABLE` pin with per-socket
  active-low enable pins.
- The current `iodefs.h` maps J1-J6 onto Octopus MOTOR0-MOTOR5 in physical
  socket order while preserving PAROL6's internal joint mapping.
- User has purchased about four EL357N-C-style optocoupler conversion modules
  described as 24 V input, 3.3 V output, NPN output. These are a better match
  than direct-wiring PAROL6 24 V metal/inductive sensors into Octopus endstop
  pins, but polarity still needs bench testing.
- User clarified that the robot has three metal/inductive limit sensors and
  three ordinary physical trigger switches. The three metal/inductive sensors
  should go through optocoupler modules. The three physical switches can likely
  connect directly to Octopus endstop inputs as dry contacts only, with no 24 V
  connected to the Octopus side.

Current mapped joint plan:

| Joint | Octopus socket | STEP | DIR | CS | ENABLE |
|---|---|---|---|---|---|
| J1 | MOTOR0 | PF13 | PF12 | PC4 | PF14 |
| J2 | MOTOR1 | PG0 | PG1 | PD11 | PF15 |
| J3 | MOTOR2 | PF11 | PG3 | PC6 | PG5 |
| J4 | MOTOR3 | PG4 | PC1 | PC7 | PA0 |
| J5 | MOTOR4 | PF9 | PF10 | PF2 | PG2 |
| J6 | MOTOR5 | PC13 | PF0 | PE4 | PF1 |

Draft limit/E-stop mapping:

| Function | Pin |
|---|---|
| J1 limit, internally LIMIT6 | PG6 |
| J2 limit, internally LIMIT2 | PG9 |
| J3 limit, internally LIMIT3 | PG10 |
| J4 limit, internally LIMIT4 | PG11 |
| J5 limit, internally LIMIT5 | PG12 |
| J6 limit, internally LIMIT1 | PG13 |
| E-stop signal | PG15 |

## Fit Assessment

### Good fit

- MCU family is close: original STM32F446RE, Octopus Pro F446 STM32F446ZE/ZET6.
- CPU frequency and flash size are compatible with the original firmware class.
- Six axes fit cleanly into the first six Octopus driver sockets.
- Octopus has enough additional driver sockets for future gripper or auxiliary
  axes, if needed.
- SPI driver control is feasible with TMC5160-style modules.
- USB CDC serial can preserve the existing high-level commander interface if the
  low-level packet protocol is kept intact.
- Current local pin mapping aligns with Klipper's official Octopus Pro V1.0
  reference pins for MOTOR0-MOTOR5.

### Not a drop-in replacement

- The original PAROL6 control board includes 24 V-compatible isolated inputs.
  Octopus endstop/DIAG inputs should be treated as MCU-level inputs unless the
  exact schematic proves otherwise. Do not wire original 24 V inductive sensors
  directly to Octopus endstop pins.
- The original board has isolated outputs; Octopus fan/heater outputs are not
  equivalent robot I/O without an interface plan.
- The original board has a smart power button / supply-hold behavior. The
  Octopus port currently maps those pins as placeholders.
- Original E-stop behavior must be recreated as hardware, not only firmware.
  A real E-stop should remove motor power or disable drives through a normally
  closed safety loop independent of MCU code.
- The original board fits the PAROL6 base; Octopus Pro is much larger and may
  require an external electronics enclosure or modified base.
- Driver module orientation, jumper mode, and `R_SENSE` must be verified on the
  actual modules before applying motor power.

## Critical Risks Before Any Motion

1. 24 V sensors and dry-contact switches
   - PAROL6 docs describe 24 V limit/sensor wiring for the original board.
   - Octopus endstop pins must not receive 24 V unless a verified interface says
     they are 24 V tolerant.
   - Recommended default for the three metal/inductive sensors: optocoupler or
     industrial input interface converting 24 V sensor signals to clean 3.3 V
     logic.
   - Recommended default for the three physical trigger switches: wire each as a
     dry contact between Octopus endstop signal and Octopus ground, using MCU
     pull-up or the board's endstop pull-up. Do not reuse the original 24 V
     switch wiring directly.
   - The purchased EL357N-C 24 V input / 3.3 V NPN output modules are suitable
     candidates for the three metal/inductive sensors.
   - If these are single-channel modules, four modules are enough for the three
     metal/inductive sensors plus one spare. They are not enough if all six
     inputs and E-stop state are intentionally optically isolated.
   - Common listings for these modules describe NPN output behavior as:
     no input signal = output high, input signal = output low. Verify this with
     a multimeter before trusting homing polarity.

2. E-stop
   - Firmware E-stop state is useful telemetry, but not a complete safety
     function.
   - Recommended default: normally closed E-stop loop that removes motor-driver
     power or drive-enable power path, plus a separate MCU input for state
     reporting.

3. Driver voltage jumpers
   - For standard TMC5160/TMC2209-style modules, use 24 V/main-power driver
     supply unless the installed modules are explicitly high-voltage rated.
   - Before power-on, photograph and document every driver voltage jumper.

4. Driver communication mode jumpers
   - TMC5160 SPI mode requires the correct Octopus jumpers.
   - BTT documentation warns that wrong jumper choices can interfere with SPI
     lines.

5. Current calibration
   - Firmware assumes `R_SENSE = 0.075`.
   - Verify the actual TMC5160 module sense resistor. If different, current
     settings will be wrong.

6. Cooling
   - PAROL6 docs require active stepper-driver cooling.
   - Octopus driver modules also need airflow, especially at PAROL6 current
     levels.

7. Homing polarity and sequence
   - Original firmware uses a mix of high-trigger and low-trigger logic:
     J1/J4/J6 differ from J2/J3/J5.
   - The EL357N-C NPN output will likely invert a 24 V active signal into a
     low-going 3.3 V input. Input polarity must be tested with a serial
     diagnostic before automatic homing.

8. Board revision mismatch
   - Octopus Pro V1.0/V1.0.1 and V1.1 have pin differences.
   - The current draft is for Octopus Pro V1.0-style pinout. Do not flash this
     onto a V1.1 target without checking the pin changes.

9. Upload path
   - SD-card firmware update expects `firmware.bin` and preserves the BTT
     bootloader.
   - DFU/direct programming can overwrite the bootloader.
   - ST-Link upload is viable for development, but the address/offset must match
     the intended boot strategy.

10. Power management placeholders
    - `SUPPLY_ON_OFF`, `SUPPLY_BUTTON_STATE`, `INPUT1`, `INPUT2`, `OUTPUT1`,
      and `OUTPUT2` are not yet electrically validated on Octopus.
    - Do not connect external actuators to those placeholder pins.

## Research-To-Build Checklist

Before firmware motion tests:

1. Identify the exact Octopus Pro board revision:
   - Board silkscreen: V1.0, V1.0.1, or V1.1.
   - MCU marking: STM32F446ZET6 expected for this plan.
   - Installed bootloader behavior: SD update to `FIRMWARE.CUR`, or direct
     ST-Link workflow.

2. Identify the exact stepper driver modules:
   - TMC5160 or other.
   - Standard-voltage or high-voltage variant.
   - Sense resistor value.
   - Required SPI jumper positions.
   - Motor voltage jumper position per socket.

3. Design the PAROL6 sensor interface:
   - Three 24 V metal/inductive sensor inputs into 3.3 V GPIO through EL357N-C
     modules.
   - Three dry-contact physical trigger switches wired to Octopus signal/GND,
     with no 24 V present.
   - Defined idle and triggered polarity.
   - Shielding/noise strategy for long robot harness wiring.
   - Diagnostic LEDs or test points, if practical.
   - Bench-test one EL357N-C module with a 24 V supply and a 3.3 V pull-up
     before connecting it to Octopus.
   - Confirm whether each PAROL6 sensor is PNP sourcing, NPN sinking, or a
     mechanical switch, then wire the optocoupler input accordingly.

4. Design the E-stop and motor power path:
   - Hardware interruption of motor power or drive enable.
   - MCU-visible E-stop state.
   - Clear recovery behavior.
   - Fuse and connector choices.

5. Create a dedicated Octopus bring-up test firmware:
   - USB serial only.
   - Print board identity/build info.
   - Test every TMC CS line with `test_connection()`.
   - Toggle each enable pin with motor power disconnected.
   - Read every limit/E-stop input with live serial output.
   - Jog one motor at very low current and speed only after input tests pass.

6. Only after the above, adapt the main PAROL6 firmware:
   - Keep the commander serial packet protocol.
   - Preserve joint order.
   - Add board-specific safety gates.
   - Add clear serial debug for failed driver communication and sensor states.

## Suggested Physical Bring-Up Order

1. USB-only power.
2. Confirm USB serial enumeration and firmware version string.
3. Confirm pin sanity with no drivers installed.
4. Bench-test one EL357N-C module off-board:
   - 24 V side powered from a current-limited supply.
   - 3.3 V side powered from a safe 3.3 V source.
   - Confirm output is high when idle and low when the 24 V input is active.
5. Connect one EL357N-C output to an Octopus endstop pin and confirm live serial
   input readings.
6. Install one driver in MOTOR0 with correct SPI and voltage jumpers.
7. Confirm SPI `test_connection()` for one driver.
8. Repeat driver communication checks for all six sockets.
9. Wire all metal/inductive sensors through optocoupler channels, not directly
   to Octopus. Wire physical switches as dry contacts only.
10. Confirm every idle/triggered polarity in serial diagnostics.
11. Confirm E-stop hardware cuts motor power or disables drivers.
12. Test one loose motor at 200-300 mA.
13. Test all six loose motors.
14. Connect robot harness with joints mechanically supported.
15. Jog each joint at low speed and low acceleration.
16. Validate limit sensors by hand before any homing command.
17. Home one controlled subset only after all inputs are confirmed.

## EL357N-C Sensor Interface Notes

The purchased EL357N-C modules should be treated as inverting isolated inputs:

| 24 V input side | 3.3 V Octopus side |
|---|---|
| No sensor/input signal | Output HIGH |
| Sensor/input active | Output LOW |

Recommended Octopus-side wiring for one channel:

| Module output pin | Connect to |
|---|---|
| VCC / V+ | Octopus 3.3 V |
| GND / 0V | Octopus GND |
| OUT / signal | Octopus endstop signal pin |

Firmware expectation for this output style:

- Configure the Octopus input as `INPUT_PULLUP`.
- A triggered sensor will usually read `LOW`.
- Set the matching joint's trigger polarity only after measuring the real signal.

Likely 24 V input wiring patterns:

| Sensor type | Wiring concept |
|---|---|
| PNP sourcing sensor | Sensor brown to +24 V, blue to 0 V, black signal to module input +, module input - to 0 V |
| NPN sinking sensor | Sensor brown to +24 V, blue to 0 V, module input + to +24 V, module input - to sensor black |
| Mechanical limit switch / physical trigger | Prefer direct Octopus dry-contact wiring: switch between endstop SIG and GND, with no 24 V on either switch lead |

Do not assume the PAROL6 sensors are one specific output type from wire colors
alone. Test one sensor with a 24 V supply and multimeter first:

- Brown to +24 V and blue to 0 V.
- Measure black-to-blue voltage in idle and triggered states.
- If black goes near +24 V when active, treat it as PNP-style for the module.
- If black is pulled near 0 V when active and floats/high otherwise, treat it as
  NPN-style for the module.
- After the module, measure OUT-to-Octopus-GND at 3.3 V side before connecting
  the robot to firmware homing.

For physical trigger switches:

- Disconnect them from any original 24 V wiring before connecting to Octopus.
- With the switch disconnected, verify continuity only: open when idle, closed
  when pressed, or vice versa.
- Wire one switch lead to the Octopus endstop signal pin and the other to
  Octopus ground.
- Configure the firmware input as `INPUT_PULLUP`, then measure whether pressed
  reads LOW or HIGH.

## Open Questions

- Which exact Octopus Pro revision is on hand?
- Which exact stepper driver modules will be installed?
- Are the PAROL6 original inductive sensors being used, or will sensors be
  replaced?
- Will the electronics live inside the PAROL6 base or in an external enclosure?
- Is SSG48 gripper support needed immediately?
- Are isolated outputs needed for the first milestone?
- Should CAN be preserved now, or deferred until base motion works over USB?

## Primary Sources

- PAROL6 GitHub:
  https://github.com/PCrnjak/PAROL6-Desktop-robot-arm
- PAROL6 official docs:
  https://source-robotics.github.io/PAROL-docs/
- PAROL6 control board docs:
  https://source-robotics.github.io/PAROL-docs/page3_1/
- PAROL6 getting started / testing docs:
  https://source-robotics.github.io/PAROL-docs/page3_2/
- PAROL6 software docs:
  https://source-robotics.github.io/PAROL-docs/page4/
- PAROL Commander:
  https://github.com/Source-Robotics/PAROL-commander-software
- PAROL6 Python API:
  https://github.com/PCrnjak/PAROL6-python-API
- BTT Octopus Pro GitHub:
  https://github.com/bigtreetech/BIGTREETECH-OCTOPUS-Pro
- BTT Octopus Pro Wiki:
  https://global.bttwiki.com/Octopus%20Pro.html
- Klipper Octopus Pro V1.0 reference config:
  https://github.com/Klipper3d/klipper/blob/master/config/generic-bigtreetech-octopus-pro-v1.0.cfg
- BTT F446 bootloader reference:
  https://github.com/bigtreetech/BIGTREETECH-OCTOPUS-V1.0/blob/master/Firmware/DFU%20Update%20bootloader/bootloader/OctoPus-F446-bootloader-32KB.hex
- Example EL357N-C module listing/spec text:
  https://business.walmart.com/ip/QSTDGVPW-EL357N-C-Optocoupler-Isolation-Module-Signal-Conversion-24V-Input-3-3V-NPN-24V-Input-3-3V-NPN-Output/20225572023
- Everlight EL357N optocoupler datasheet:
  https://www.everlighteurope.com/custom/files/datasheets/DPC-0000014.pdf
