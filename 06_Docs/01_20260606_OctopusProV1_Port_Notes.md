# Octopus Pro V1.0 F446 PAROL6 Firmware Port Notes

> **폐기된 초기 초안 / SUPERSEDED DRAFT**
>
> 이 문서의 MOTOR0–MOTOR5 배선은 최종 실물 구성과 다릅니다. 실제 배선에는
> [`02_20260722_Final_Wiring_and_Pin_Map.md`](02_20260722_Final_Wiring_and_Pin_Map.md)를
> 사용하세요.

## Status

- Working copy: `03_Board_Firmware/01_20260606_OctopusProV1_Port__InProgress`
- Build output: `07_Output/01_20260606_OctopusProV1_Firmware__Draft/parol6_octopus_pro_v1_f446_bootloader32k_draft.bin`
- Build command: `uvx platformio run`
- Build result: success
- Link offset: `0x08008000`, preserving the BTT 32KiB bootloader

## Sources Used

- BTT Octopus Pro repository: https://github.com/bigtreetech/BIGTREETECH-OCTOPUS-Pro
- BTT Octopus Pro hardware pin PDF in that repository: `Hardware/BIGTREETECH Octopus Pro - PIN.pdf`
- Klipper Octopus Pro V1.0 reference config: https://github.com/Klipper3d/klipper/blob/master/config/generic-bigtreetech-octopus-pro-v1.0.cfg

## Driver Mapping

PAROL6's original code keeps a non-linear internal mapping:

```text
J1 = PUL1 / DIR1 / SELECT1
J2 = PUL6 / DIR6 / SELECT6
J3 = PUL5 / DIR5 / SELECT5
J4 = PUL4 / DIR4 / SELECT4
J5 = PUL2 / DIR2 / SELECT2
J6 = PUL3 / DIR3 / SELECT3
```

The port preserves that software structure while assigning joints to Octopus driver sockets in physical order:

| Joint | Octopus socket | STEP | DIR | CS | ENABLE |
|---|---|---|---|---|---|
| J1 | MOTOR0 | `PF13` | `PF12` | `PC4` | `PF14` |
| J2 | MOTOR1 | `PG0` | `PG1` | `PD11` | `PF15` |
| J3 | MOTOR2 | `PF11` | `PG3` | `PC6` | `PG5` |
| J4 | MOTOR3 | `PG4` | `PC1` | `PC7` | `PA0` |
| J5 | MOTOR4 | `PF9` | `PF10` | `PF2` | `PG2` |
| J6 | MOTOR5 | `PC13` | `PF0` | `PE4` | `PF1` |

SPI remains on:

```text
MISO = PA6
MOSI = PA7
SCK  = PA5
```

## Limit And E-Stop Draft Mapping

| Function | Pin |
|---|---|
| J1 limit, internally `LIMIT6` | `PG6` |
| J2 limit, internally `LIMIT2` | `PG9` |
| J3 limit, internally `LIMIT3` | `PG10` |
| J4 limit, internally `LIMIT4` | `PG11` |
| J5 limit, internally `LIMIT5` | `PG12` |
| J6 limit, internally `LIMIT1` | `PG13` |
| E-stop | `PG15` |

Inputs are currently initialized as `INPUT_PULLUP`. This suits open-collector or optocoupler outputs that pull the signal low when active, but the homing trigger polarity must be verified before running automatic homing.

## Important Hardware Notes

- Remove any DIAG jumpers that would connect TMC driver DIAG pins to endstop inputs if wired sensors are used.
- Do not connect official 24V inductive sensors directly to Octopus endstop pins. Use a 24V-to-3.3V optocoupler input module.
- Non-motion I/O such as `SUPPLY_ON_OFF`, `INPUT1`, `INPUT2`, `OUTPUT1`, and `OUTPUT2` is only placeholder-mapped to EXP header pins. Do not wire external actuators to those pins yet.
- This draft does not validate USB enumeration, TMC5160 SPI communication, motor direction, sensor polarity, or homing behavior on real hardware.

## Next Physical Bring-Up Order

1. Power the Octopus board from USB only and confirm firmware upload/USB serial.
2. With 24V motor power disconnected, confirm all six TMC5160 CS lines respond.
3. Confirm each enable pin disables/enables the expected driver socket.
4. Wire one sensor through an optocoupler and confirm idle/triggered logic in firmware.
5. Test one motor at low current and low speed before connecting the robot drivetrain.
6. Only after all six sensors read correctly, attempt homing.
