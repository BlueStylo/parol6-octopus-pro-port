# PAROL6 Octopus Pro F446 Firmware — Exp23 Final

PAROL6 원본 low-level control-board firmware를 **BTT Octopus Pro
V1.0/V1.0.1 F446**에서 실행하기 위한 비공식 minimal port입니다. 실제 검증은
Octopus Pro V1.0.1 F446 한 대에서 수행했습니다.

이 source tree는 이전 실험 firmware에 기능을 계속 쌓은 버전이 아닙니다.
고정한 PAROL6 upstream firmware에서 다시 시작하여 원본 motion, HOME,
protocol과 motor profile을 보존하고 Octopus에 반드시 필요한 board adapter만
추가했습니다. Exp23은 여기에 원본 pneumatic port 1의 `OUTPUT1`을
`FAN0/PA8`로 연결합니다.

## 상태

- Clean STM32F446ZE build: PASS
- Static/protocol/pneumatic regression: PASS, 21 tests
- Native Octopus/STOP7 regression: PASS
- 원본 first/repeat HOME native regression: PASS
- 실제 6축 motion, limit, STOP7, first/repeat HOME와 Waldo 프로그램: PASS
- FAN0 software/protocol path: PASS
- 실제 FAN0 24 V 솔레노이드 및 공압 전환: **NOT FORMALLY VERIFIED**

이 프로젝트는 Source Robotics 또는 PAROL6 공식 controller 제품과 관련 없는
개인 포트이며 인증된 안전 controller가 아닙니다.

## 지원 하드웨어

- BTT Octopus Pro V1.0/V1.0.1 **F446 only**
- STM32F446ZET6, 12 MHz HSE, 180 MHz core
- BTT 32 KiB bootloader, application address `0x08008000`
- BTT TMC5160T × 6, shared SPI with per-socket CS/EN
- DC 24 V main/motor supply used by the validated build

H723 또는 핀맵이 다른 보드에는 이 firmware를 flash하지 마세요.

## 고정 upstream

```text
repository:
https://github.com/PCrnjak/PAROL6-Desktop-robot-arm.git

pinned commit:
896fa9fd793a076a0b3790b8a4a29a6f5bb12fcb

firmware directory:
PAROL6 control board main software

pinned firmware tree:
2bac41ef589ae8e253384a977c83b8526726becc
```

세부 provenance는 [UPSTREAM.lock](UPSTREAM.lock), 허용된 차이는
[MINIMAL_DELTA.md](MINIMAL_DELTA.md), 결과는 [VALIDATION.md](VALIDATION.md)에
기록했습니다.

## 보존한 원본 동작

- 56-byte PC command frame / 60-byte board response frame
- 원본 command 번호와 dispatch 의미
- `reset_homing()`과 `home_all()` 본문
- 원본 J1–J6 감속비, microstep 32, software range, home/standby 좌표
- 원본 TMC5160 RMS/hold/StealthChop profile
- 원본 limit trigger level `LOW/HIGH/HIGH/LOW/HIGH/LOW`
- 원본 HOME 순서 J1–J3 → J4 → J6 → J5

원본 HOME 본문에 별도의 timeout, 초기 sensor plausibility gate 또는 새로운
stage 조건을 삽입하지 않았습니다.

## Octopus 배선 계약

| Joint | Motor | Limit | Active |
| --- | --- | --- | --- |
| J1 | MOTOR6 | STOP3 / PG11 | LOW |
| J2 | MOTOR5 | STOP2 / PG10 | HIGH |
| J3 | MOTOR4 | STOP1 / PG9 | HIGH |
| J4 | MOTOR3 | STOP4 / PG12 | LOW |
| J5 | MOTOR1 | STOP0 / PG6 | HIGH |
| J6 | MOTOR0 | STOP5 / PG13 | LOW |

MOTOR2는 병렬 Z 출력이어서 사용하지 않고 MOTOR7도 비웁니다. 상세 STEP,
DIR, CS, EN 및 센서 배선은
[최종 배선표](../../06_Docs/02_20260722_Final_Wiring_and_Pin_Map.md)를
참조하세요.

## STOP7 adapter

STOP7 `PG15`는 이 port의 유일한 추가 motion policy입니다. NC loop가 닫히면
LOW, 버튼 누름 또는 단선은 HIGH입니다. HIGH가 되면 모든 STEP을 차단하고
현재 동작과 HOME을 취소하지만 이미 enable된 driver EN은 유지하여 holding
torque를 남깁니다.

접점이 닫혔다고 자동 재개하지 않습니다. Waldo Reset
`255 → 103 → 255` 뒤 별도 `101 ENABLE`이 필요합니다. STOP7이 열린 채
부팅하면 driver를 enable하지 않습니다. 이는 hard E-stop이 아니므로 별도의
24 V 직접 차단 수단이 필요합니다.

## FAN과 pneumatic output

- FAN4 `PD14`, FAN5 `PD15`: 부팅 시 HIGH
- FAN6/FAN7: V_FAN 인가 시 상시 동작
- `OUTPUT1 → FAN0/PA8`
- pneumatic open: HIGH/ON
- pneumatic close: LOW/OFF
- boot/reset: LOW/OFF
- `OUTPUT2 → PE15`, logic-level spare

STOP7, Disable 또는 USB 단절은 FAN0을 강제로 OFF하지 않습니다. 전원이
유지되는 동안 마지막 명령 상태가 남고, 메인보드 24 V 차단 또는 재부팅
때 OFF가 됩니다. 실제 valve 작동은 정식 검증 완료 항목이 아닙니다.

## 지원하지 않는 원본 보드 기능

- 원본 PB0 bus-voltage ADC
- 원본 soft power latch
- 원본 serial flash runtime
- 원본 CAN/gripper hardware

CAN API와 packet field는 call graph 호환을 위해 남아 있지만 실제 CAN
hardware adapter는 no-op입니다.

## 빌드

기준 환경:

```text
PlatformIO Core 6.1.19
ST STM32 platform 19.6.0
Arduino STM32 framework 2.12.0
GNU Arm Embedded GCC 12.3.1
```

명령:

```bash
pio run -e btt_octopus_pro_v1_0_f446ze_literal_upstream
```

출력:

```text
.pio/build/btt_octopus_pro_v1_0_f446ze_literal_upstream/firmware.bin
```

검증된 release binary:

```text
size: 50156 bytes
SHA-256:
66471679470987c078078b68e6dd05ace0703d8d5a25a77b6efaf81afdf7fee8
vector/application address: 0x08008000
```

전체 플래시와 최초 기동 순서는
[빌드, 플래시 및 최초 기동](../../06_Docs/03_20260722_Build_Flash_and_First_Start.md)을
따르세요.

## 테스트 재실행

보드 adapter의 독립 실행 가능한 시험은 추가 checkout 없이 실행됩니다.

```bash
python3 -m unittest tests.test_hardware_contract
bash tests/run_octopus_board_native_tests.sh
```

원본 소스 동일성 및 host protocol 전체 시험에는 고정 upstream checkout을
환경변수로 지정합니다. 해당 변수가 없으면 외부 checkout이 필요한 시험만
명시적으로 skip됩니다.

```bash
export PAROL6_UPSTREAM_FIRMWARE_DIR=/path/to/PAROL6-Desktop-robot-arm/'PAROL6 control board main software'
export PAROL6_PYTHON_API_DIR=/path/to/PAROL6-python-API-v0.3.1
python3 -m unittest discover -s tests -p 'test_*.py'
bash tests/run_octopus_board_native_tests.sh
```

Python API 호환 시험에는 `numpy`가 필요합니다.

## Protocol compatibility

원본 packet contract를 유지하므로 Waldo Web Commander, PAROL6 Python API와
공식 PAROL Commander의 기존 serial protocol을 사용할 수 있습니다. 주 실기
검증 환경은 Waldo였습니다. 한 번에 하나의 controller만 serial port를
열어야 합니다.

Exp23의 USB manufacturer string은 검증된 binary identity를 유지하기 위해
내부적으로 `PAROL6_OCTOPUS_EXP22`로 남아 있습니다. 기능에는 영향이 없지만
USB 장치 문자열만으로 Exp22/Exp23을 구분하면 안 됩니다.

## English summary

This is a GPLv3, literal-upstream minimal port of the PAROL6 low-level firmware
for BTT Octopus Pro V1.0/V1.0.1 F446 boards. It preserves the original PAROL6
56/60-byte protocol, homing routine, coordinates and TMC5160 profiles while
adapting the Octopus clock, pins, per-socket CS/EN, unsupported peripherals and
an NC STOP7 torque-hold latch. Exp23 additionally maps pneumatic port 1 to
FAN0/PA8. Six-axis motion, limits, STOP7, repeated homing and Waldo programs
were observed working on one V1.0.1 F446 build. Physical FAN0 solenoid and
pneumatic actuation have not been formally verified.

## 라이선스

이 derivative firmware는 root [GPLv3 license](LICENSE)를 따릅니다. 포함된
third-party library는 각 library 디렉터리의 고지를 따릅니다. 원본 저자와
PAROL6 프로젝트의 저작권 및 attribution을 유지하세요.
