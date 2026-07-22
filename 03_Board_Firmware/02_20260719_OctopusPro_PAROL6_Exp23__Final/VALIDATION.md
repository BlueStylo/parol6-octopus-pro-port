# Exp23 Final Validation

## Release verdict

이 source tree와 release binary는 BTT Octopus Pro V1.0/V1.0.1 F446용 PAROL6
minimal port입니다. 오프라인 회귀와 clean build를 통과했고, 프로젝트 제작자의
Octopus Pro V1.0.1 F446 로봇에서 전체 6축 동작, sensor, STOP7 torque-hold,
최초 및 반복 HOME, Waldo 프로그램 실행이 관찰됐습니다.

판정은 **hardware-validated experimental firmware**입니다. 인증된 safety
controller나 모든 PAROL6 조립체에 대한 drop-in guarantee를 의미하지
않습니다. FAN0 software path는 검증됐지만 실제 DC 24 V valve와 공압 전환은
정식 검증 완료로 기록하지 않습니다.

## Artifact identity

```text
target:
btt_octopus_pro_v1_0_f446ze_literal_upstream

firmware.bin size:
50156 bytes

firmware.bin SHA-256:
66471679470987c078078b68e6dd05ace0703d8d5a25a77b6efaf81afdf7fee8

MCU: STM32F446ZET6
HSE: 12 MHz
core clock: 180 MHz
vector/application address: 0x08008000
ELF entry point: 0x08012499
USB VID:PID: 0483:5740
USB CDC monitor baud: 3000000
USB manufacturer string: PAROL6_OCTOPUS_EXP22
```

USB string은 Exp22에서 Exp23으로 기능을 한정 변경하면서 그대로 보존된
식별자입니다. Exp23 여부는 release hash로 판단해야 합니다.

## Build environment

```text
PlatformIO Core: 6.1.19
ST STM32 platform: 19.6.0
Arduino STM32 framework: 2.12.0
GNU Arm Embedded GCC: 12.3.1

RAM: 8188 / 131072 bytes (6.2%)
Flash sections: 49640 / 524288 bytes (9.5%)
```

공개 source tree의 clean build와 보관한 release binary가 동일한 SHA-256을
갖는 것을 확인했습니다.

## Upstream identity

```text
repository:
https://github.com/PCrnjak/PAROL6-Desktop-robot-arm.git

pinned commit:
896fa9fd793a076a0b3790b8a4a29a6f5bb12fcb

pinned firmware tree:
2bac41ef589ae8e253384a977c83b8526726becc
```

원본 `constants`, `common`, `motor_init`, `structs`, `utils` 핵심 파일과
protocol/HOME 영역에 대한 identity 검사가 통과했습니다. 자세한 허용 diff는
[MINIMAL_DELTA.md](MINIMAL_DELTA.md)에 있습니다.

## Automated validation

| Check | Result |
| --- | --- |
| Python static/protocol/pneumatic tests | PASS — 21 tests |
| Octopus native GPIO/STOP7 tests | PASS |
| Production `reset_homing()` native regression | PASS |
| Production `home_all()` first HOME regression | PASS |
| Production `home_all()` repeated HOME regression | PASS |
| Upstream source/delta contract | PASS |
| Final motor/limit mapping contract | PASS |
| STOP7 boot, interrupt, reset and EN rollback contract | PASS |
| FAN0 boot LOW and OUTPUT1 routing contract | PASS |
| Clean F446ZE build | PASS |
| Binary vector, entry, size and SHA audit | PASS |
| Exp22-to-Exp23 binary delta | PASS — exactly 5 bytes |

Production function hashes:

```text
reset_homing:
2116c84242001755e564efd2f8cc052e53e73a8a47678bedc955b459d478606b

home_all:
afdcc71b3a22710239f2fcfd84882687fd5b8dbc7b45ce1dfe155affd2db9c1f
```

Exp22와 Exp23은 모두 50,156 bytes이며 5-byte 차이는 runtime OUTPUT1 write와
boot-safe LOW preload에서 pin alias가 `PE14 → PA8`로 바뀐 결과와 일치합니다.

## Operator-observed hardware validation

| Check | Result |
| --- | --- |
| SD bootloader update / `FIRMWARE.CUR` workflow | PASS |
| USB CDC and 56/60-byte communication | PASS |
| J1–J6 TMC5160T initialization and holding torque | PASS |
| J1–J6 direction/motion | PASS |
| All six limit inputs and HOME directions | PASS |
| MOTOR2 bypass and final six-socket mapping | PASS |
| STOP7 during J6 motion | PASS — immediate motion stop observed |
| STOP7 holding torque | PASS — torque remained after stop |
| STOP7 release without auto-resume | PASS |
| Reset then separate Enable recovery | PASS |
| First full six-axis HOME | PASS |
| Repeated full six-axis HOME | PASS |
| Waldo live control and sample program | PASS |
| Four 12 V 4040 fans | PASS |
| FAN0 no-load voltage measurement | NOT FORMALLY RECORDED |
| FAN0 solenoid click/electrical actuation | NOT FORMALLY VERIFIED |
| Pneumatic gripper under pressure/load | NOT FORMALLY VERIFIED |

이 표의 hardware PASS는 제작자의 commissioning 관찰 기록입니다. 보드 flash
read-back, 인증기관 시험 또는 독립 반복시험을 뜻하지 않습니다.

## Final hardware contract

```text
JOINT -> MOTOR:
J1/J2/J3/J4/J5/J6 -> M6/M5/M4/M3/M1/M0

JOINT -> LIMIT:
J1/J2/J3/J4/J5/J6 -> STOP3/STOP2/STOP1/STOP4/STOP0/STOP5

LIMIT ACTIVE LEVEL:
LOW/HIGH/HIGH/LOW/HIGH/LOW

UNUSED:
MOTOR2, MOTOR7, STOP6

STOP7:
PG15, NC closed LOW, open/wire-break HIGH
```

자세한 pin table은
[최종 배선 및 핀맵](../../06_Docs/02_20260722_Final_Wiring_and_Pin_Map.md)에
있습니다.

## STOP7 boundary

STOP7은 STEP pulse를 차단하고 HOME/motion을 취소하며 이미 enable된 driver의
holding torque를 보존합니다. release만으로 재개하지 않으며 Reset 후 별도
Enable이 필요합니다. STOP7 open boot에서는 driver가 enable되지 않습니다.

이 검증은 firmware behavior에 한정됩니다. STOP7은 인증된 hardware E-stop이
아니며 24 V 직접 차단 수단을 대체하지 않습니다.

## Unsupported and unverified

- Octopus Pro H723 및 다른 pinout
- 원본 PB0 bus-voltage ADC와 soft power latch
- 원본 serial flash runtime
- CAN transceiver와 CAN gripper hardware
- OUTPUT2 power-load use
- FAN0 physical valve and pneumatic load operation
- PSU sizing across arbitrary payloads and duty cycles
- long-duration thermal, EMC and lifecycle testing
- certified functional safety behavior

FAN0은 STOP7, Disable 또는 USB/backend 단절 시 마지막 상태를 유지하고,
메인보드 24 V 차단이나 재부팅 때 OFF가 됩니다. 이는 명시해야 하는 동작
계약이지 pneumatic fail-safe 검증이 아닙니다.

## Re-run commands

Upstream identity와 Waldo/Python API 검사를 포함한 전체 21개 test를 실행하려면
고정 upstream firmware와 PAROL6 Python API v0.3.1 checkout을 각각 환경
변수로 지정해야 합니다. Python 환경에는 `numpy`가 필요합니다.

```bash
export PAROL6_UPSTREAM_FIRMWARE_DIR=/path/to/PAROL6-Desktop-robot-arm/'PAROL6 control board main software'
export PAROL6_PYTHON_API_DIR=/path/to/PAROL6-python-API
python3 -m unittest discover -s tests -p 'test_*.py'
bash tests/run_octopus_board_native_tests.sh
python3 tests/run_upstream_home_native_tests.py
pio run -e btt_octopus_pro_v1_0_f446ze_literal_upstream
```

두 환경 변수가 없으면 upstream/API 의존 검사는 명시적으로 skip됩니다.

실기 재검증은
[빌드, 플래시 및 최초 기동](../../06_Docs/03_20260722_Build_Flash_and_First_Start.md)의
순서를 따르세요.

## English summary

The exact release binary and clean source build pass protocol, pneumatic
routing, STOP7, upstream-delta, original first/repeat homing and binary audits.
On one Octopus Pro V1.0.1 F446 arm, six-axis motion, all limits, STOP7 with
holding torque, first and repeated full homing, Waldo programs and four cooling
fans were observed working. This is experimental hardware validation, not a
certified safety result. Physical FAN0 24 V solenoid and pneumatic actuation
remain not formally verified.
