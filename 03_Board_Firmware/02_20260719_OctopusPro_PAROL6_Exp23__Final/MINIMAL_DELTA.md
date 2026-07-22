# Exp23 minimal-delta contract

## 기준

비교 기준은 `UPSTREAM.lock`의 firmware subtree다. Exp23의 원칙은
“원본 motion/controller를 새로 구현”하는 것이 아니라 “원본이 Octopus
pin과 clock을 사용하게 하는 것”이다.

## 의도된 diff inventory

### 수정 파일

| 파일 | 허용된 차이 |
|---|---|
| `platformio.ini` | F446ZE/32 KiB bootloader target, 12 MHz HSE, USB CDC 및 링크할 source 목록 |
| `src/iodefs.h` | joint별 STEP/DIR/CS/EN, limit, SPI, fan, optional I/O pin alias |
| `src/main.cpp` | `octopus_board.h` include와 표시된 `OCTOPUS_ADAPTER_BEGIN/END` hook만 추가·대체 |

### 추가 파일

| 파일 | 역할 |
|---|---|
| `boards/genericSTM32F446ZE.json` | STM32F446ZET6, 512 KiB flash, 32 KiB BTT bootloader offset |
| `src/octopus_clock.cpp` | Octopus의 12 MHz HSE에서 180 MHz core와 48 MHz USB clock 구성 |
| `src/octopus_board.h/.cpp` | 개별 socket EN/CS, GPIO preload, final mapping, fan과 STOP7 adapter |
| `src/octopus_can_stub.cpp` | 미검증 CAN hardware를 건드리지 않는 no-op API adapter |

## 문자 그대로 보존하는 핵심

다음 파일은 pinned upstream과 byte-for-byte 동일해야 한다.

- `src/constants.h`
- `src/common.h`
- `src/motor_init.h`
- `src/motor_init.cpp`
- `src/structs.h`
- `src/structs.cpp`
- `src/utils.h`
- `src/utils.cpp`

다음 `main.cpp` 영역도 upstream 본문을 보존한다.

- PC frame parser와 `Unpack_data()`
- `Pack_data()` 및 56-byte payload/60-byte total response layout
- 원본 command dispatch
- `reset_homing()`
- `home_all()` 전체 본문
- `Init_motor_direction()`과 `Init_motor_drivers()`의 원본 TMC 설정

`main.cpp`에서 기존 줄을 대규모로 편집하는 대신 adapter hook 경계를
명시한다. STOP7 hook을 제외한 controller 동작 변경은 허용하지 않는다.

## Octopus에서만 필요한 변환

### 비선형 upstream symbol order

upstream의 `driver[]`/`stepper[]` 생성자는 pin symbol을
J1=`1`, J2=`6`, J3=`5`, J4=`4`, J5=`2`, J6=`3` 순서로 사용한다.
`iodefs.h`는 이 구조를 건드리지 않고 symbol 자체를 다음 socket으로
보낸다.

| Joint | Upstream pin symbol | Octopus socket |
|---|---:|---:|
| J1 | 1 | MOTOR6 |
| J2 | 6 | MOTOR5 |
| J3 | 5 | MOTOR4 |
| J4 | 4 | MOTOR3 |
| J5 | 2 | MOTOR1 |
| J6 | 3 | MOTOR0 |

### 원본 global enable 대체

원본 PCB의 단일 `GLOBAL_ENABLE` 대신 Octopus socket마다 active-low EN이
있다. 초기화 전에는 모든 EN을 HIGH로 preload하고, TMC SPI 검사가 끝난
뒤 매핑된 여섯 socket만 LOW로 enable한다. MOTOR2와 MOTOR7은 항상
disable한다.

### 공유 SPI

모든 TMC5160과 MAX31865 socket이 PA5/PA6/PA7을 공유하므로 각 transaction
사이에 모든 CS를 HIGH로 되돌리고 TMC 속도를 1 MHz로 제한한다. 원본의
current, microstep, StealthChop, `toff`, `blank_time`, autoscale setter는
그대로 실행한다.

### OUTPUT1의 FAN0 전력 출력 매핑

원본 56-byte PC frame의 I/O bit 2, firmware의 `commanded_OUT1` 및
Waldo/PAROL6 Python API의 pneumatic port 1 의미는 그대로 유지한다.
Octopus에서 코일을 직접 구동할 수 있도록 `OUTPUT1` pin alias만
원본 EXP logic pin 대신 `FAN0(PA8)`의 active-HIGH MOSFET 입력으로
보낸다.

- `LOW`: FAN0 MOSFET OFF, 단일 솔레노이드 spring-return/close
- `HIGH`: FAN0 MOSFET ON, open
- 부팅 전 pin preload: LOW
- `OUTPUT2`: 기존 PE15 logic-level spare 유지
- STOP7: motion latch만 담당하며 OUTPUT1을 강제 변경하지 않음

### 보드에 없는 주변회로

- 원본 PB0 ADC bus-voltage 측정: 링크하지 않음
- 원본 soft supply latch: 호출하지 않음
- CAN/gripper bus: packet field는 유지, hardware API는 no-op
- 원본 serial flash: 링크하지 않음
- 원본 `hw_init.cpp`: `octopus_board.cpp`의 동명 digital init 함수로 대체

## 하나의 정책 예외

`src/octopus_board.cpp`의 STOP7 NC torque-hold latch만 upstream에 없는
정책이다. 이 예외는 `home_all()` 안으로 섞지 않고 loop 시작, command
처리 직후, valid frame parse 직후와 IO[4] 상태 보고 hook으로 격리한다.

STOP7 ISR은 STEP을 LOW로 만든 뒤 여덟 STEP GPIO를 input-pulldown으로
gate하므로 원본 motion 함수가 현재 loop를 끝내더라도 pulse가 물리
출력되지 않는다. 활성 EN은 유지한다. Waldo의
`255 -> 103 -> 255`가 IO[4] reset acknowledgement를 만들고, 별도의
`101`이 upstream dispatch에 도달해야 gate를 해제한다. 부팅 시 STOP이
열렸거나 enable/restore 중 fault가 생기면 mapped EN은 HIGH로 rollback
한다. 이 예외 이외에 원본 HOME 조건, 시간 제한, sensor 순서 또는
current profile을 바꾸지 않는다.

## 금지되는 drift

Exp23 검증은 다음 변화가 생기면 실패해야 한다.

- `motor_init.cpp`의 current, hold, trigger, reduction, offset 또는 range 변경
- `home_all()`의 stage, speed, target, sensor 조건 또는 순서 변경
- protocol frame byte offset/길이 변경
- MOTOR2 재사용 또는 MOTOR7 자동 활성화
- STOP7 release만으로 자동 재개
- 추가 timeout/plausibility gate를 원본 HOME 본문에 삽입
- CAN/ADC/power-latch를 물리 검증 없이 다시 링크
