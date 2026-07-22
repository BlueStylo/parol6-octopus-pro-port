# PAROL6 Octopus Pro 최종 배선 및 핀맵

이 문서는 실제 제작에 사용한 **BTT Octopus Pro V1.0.1 F446**와 Exp23
펌웨어의 최종 배선 계약을 기록합니다. 소스는 Octopus Pro V1.0/V1.0.1
F446 핀맵을 대상으로 하지만, 실제 하드웨어 검증은 V1.0.1 F446에서
수행했습니다. H723 보드나 다른 핀맵의 보드에는 사용하지 마세요.

> **주의:** 이 문서는 인증된 전기·기계 안전 설계가 아닙니다. 배선 변경과
> 점퍼 이동은 24 V와 USB를 모두 분리한 상태에서만 하세요. 로봇이 움직일
> 때는 별도의 24 V 직접 차단 수단을 즉시 조작할 수 있어야 합니다.

## 전원과 드라이버

- `MB_POWER_IN`: DC 24 V
- `MOTOR_POWER_IN`: DC 24 V
- 드라이버: BTT TMC5160T, SPI mode
- 사용 소켓: MOTOR0, MOTOR1, MOTOR3, MOTOR4, MOTOR5, MOTOR6
- 미사용 소켓: MOTOR2, MOTOR7
- 각 사용 소켓의 전압 선택 점퍼는 `MOTOR_POWER` 쪽으로 설정합니다.
- 각 사용 소켓은 `MISO / CS / SCK / MOSI` 네 개의 SPI 점퍼를 사용합니다.
- MOTOR2는 Octopus의 두 Z 모터 출력이 병렬로 연결된 위치이므로 이 포트에서
  사용하지 않습니다. 펌웨어도 MOTOR2와 MOTOR7의 EN을 비활성 상태로
  유지합니다.

24 V 극성을 보드 실크와 멀티미터로 확인한 뒤 연결하세요. 전원이 들어간
상태에서 드라이버, 모터 케이블 또는 전압 선택 점퍼를 꽂거나 빼지 마세요.

## 관절, 모터, 리미트 입력

| Joint | Motor socket | STEP | DIR | CS | EN | Limit connector | Signal pin | Trigger level | 검증된 HOME 방향 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| J1 | MOTOR6 | PE2 | PE3 | PE1 | PD4 | STOP3 | PG11 | LOW | N |
| J2 | MOTOR5 | PC13 | PF0 | PE4 | PF1 | STOP2 | PG10 | HIGH | P |
| J3 | MOTOR4 | PF9 | PF10 | PF2 | PG2 | STOP1 | PG9 | HIGH | P |
| J4 | MOTOR3 | PG4 | PC1 | PC7 | PA0 | STOP4 | PG12 | LOW | P |
| J5 | MOTOR1 | PG0 | PG1 | PD11 | PF15 | STOP0 | PG6 | HIGH | N |
| J6 | MOTOR0 | PF13 | PF12 | PC4 | PF14 | STOP5 | PG13 | LOW | N |

공유 SPI 신호는 SCK `PA5`, MISO `PA6`, MOSI `PA7`입니다. 모든 드라이버는
공유 버스를 사용하지만 CS와 active-low EN은 소켓마다 분리되어 있습니다.

### 모터 코일 확인

케이블 색상은 제조사마다 다르므로 색만 보고 연결하지 마세요. 전원을 모두
분리하고 멀티미터의 연속성 또는 저항 모드로 두 코일 쌍을 찾습니다.

- 한 코일의 두 선 → `A1/A2`
- 다른 코일의 두 선 → `B2/B1`
- `A1/B1`처럼 서로 다른 코일을 한 쌍으로 연결하면 모터가 정상 회전하지
  않고 떨리거나 두두두 소리만 날 수 있습니다.

회전 방향이 반대라면 전원이 꺼진 상태에서 한 코일 쌍의 두 선만 서로
바꾸거나 펌웨어 방향 설정을 검토하세요.

## 센서 구성

실제 제작에서 사용한 센서 종류는 다음과 같습니다.

| Joint | Sensor type | Electrical behavior |
| --- | --- | --- |
| J1 | DC 24 V 금속 근접센서 + 절연 인터페이스 | 감지 시 MCU 입력 LOW |
| J2 | NC 기계식 스위치 | 정상 폐회로, 작동/단선 시 입력 HIGH |
| J3 | NC 기계식 스위치 | 정상 폐회로, 작동/단선 시 입력 HIGH |
| J4 | DC 24 V 금속 근접센서 + 절연 인터페이스 | 감지 시 MCU 입력 LOW |
| J5 | NC 기계식 스위치 | 정상 폐회로, 작동/단선 시 입력 HIGH |
| J6 | DC 24 V 금속 근접센서 + 절연 인터페이스 | 감지 시 MCU 입력 LOW |

24 V 센서 출력을 STOP 입력에 직접 연결하면 안 됩니다. EL357N 계열처럼
센서 측과 MCU 측이 분리된 인터페이스를 사용하고, 인터페이스의 정확한
단자 배열을 해당 모듈 회로도로 확인하세요. MCU 측에서는 STOP의 signal과
GND 사이에 **3.3 V 입력에 안전한 접점 또는 open-collector 동작**만
나와야 합니다. OUT과 GND를 뒤바꾸면 센서가 계속 고정 상태로 보일 수
있습니다.

STOP6 `PG14`는 예비 입력입니다.

## STOP7 torque-hold 정지 입력

| Function | Connector | Pin | Normal | Stop/wire break |
| --- | --- | --- | --- | --- |
| Firmware torque-hold stop | STOP7 | PG15 | NC loop closed, LOW | Open, HIGH |

STOP7은 정상 상태에서 signal과 GND가 연결되는 NC 접점을 사용합니다. 버튼을
누르거나 선이 끊어져 HIGH가 되면 펌웨어는 STEP 출력을 즉시 차단하고 진행
중인 동작과 HOME을 취소합니다. 이미 활성화된 드라이버의 EN은 유지하므로
로봇은 정지 토크를 유지합니다.

접점이 다시 닫혀도 자동으로 움직이지 않습니다. Waldo에서 Reset
(`IDLE → CLEAR → IDLE`)을 수행한 뒤 별도로 Enable해야 합니다. STOP7이
열린 채 부팅하면 드라이버는 처음부터 enable되지 않으므로 정지 토크가
생기지 않습니다.

STOP7은 firmware 기능이며 인증된 hard E-stop이 아닙니다. 별도의 24 V
직접 차단 장치를 유지하세요.

## 12 V 냉각팬 네 개

케이스는 40 × 40 mm(4040) DC 12 V 팬 네 개를 FAN4–FAN7에 장착하도록
설계했습니다.

| Port | MCU behavior |
| --- | --- |
| FAN4 | PD14, 부팅 시 HIGH |
| FAN5 | PD15, 부팅 시 HIGH |
| FAN6 | V_FAN 인가 시 상시 동작 |
| FAN7 | V_FAN 인가 시 상시 동작 |

FAN6/FAN7은 개별 MCU 제어 출력이 아닙니다. 12 V 팬을 연결하기 전에 팬
전압 선택 점퍼가 반드시 **12 V rail**에 있는지 확인하세요. 24 V VIN이
선택된 출력에 12 V 팬을 연결하면 팬이 손상될 수 있습니다.

케이스 STL은
[04_3D_Printing/01_Octopus_Pro_Board_Enclosure](../04_3D_Printing/01_Octopus_Pro_Board_Enclosure/README.md)에
있습니다.

## FAN0 DC 24 V 단일 솔레노이드

| Function | Port | Pin | OFF | ON |
| --- | --- | --- | --- | --- |
| PAROL6 OUTPUT1 / pneumatic port 1 | FAN0 | PA8 | LOW, close/spring-return | HIGH, open |

- 모든 전원을 분리한 상태에서 FAN0 전압 점퍼를 `VIN/V_FUSED`에 둡니다.
- 메인보드 VIN이 24 V일 때 FAN0도 24 V 출력이 됩니다.
- 코일 `+`는 FAN0 `+`, 코일 `-`는 FAN0 `-`에 연결합니다.
- PWM이 아니라 완전 ON/OFF만 사용합니다.
- 부팅 또는 메인보드 재부팅 시 FAN0은 OFF입니다.
- STOP7, Disable 또는 USB 단절은 FAN0을 자동으로 OFF시키지 않고 마지막
  명령 상태를 유지합니다.
- UI 상태는 출력 명령의 echo이며 실제 전압이나 공압 전환 피드백이 아닙니다.

FAN0 소프트웨어 경로와 정적 회귀는 검증됐지만 **실제 솔레노이드 코일 및
공압 전환은 정식 완료 기록이 없습니다.** 첫 시험은 코일을 분리한 상태에서
멀티미터로 `close≈0 V`, `open≈24 V`, `close≈0 V`를 확인한 뒤 진행하세요.

## English summary

The validated board is a BTT Octopus Pro V1.0.1 F446. The final motor map is
J1–J6 → MOTOR6/MOTOR5/MOTOR4/MOTOR3/MOTOR1/MOTOR0; MOTOR2 and MOTOR7 are
unused. J1/J4/J6 use isolated 24 V proximity sensors, while J2/J3/J5 use NC
mechanical switches. STOP7 is an NC, active-HIGH-on-open firmware stop that
blocks STEP pulses while preserving already-enabled holding torque. It is not
a certified hardware emergency stop. FAN4–FAN7 power four 12 V 4040 fans;
FAN0 maps PAROL6 pneumatic port 1 to a 24 V single-solenoid valve. Physical
FAN0 valve actuation has not been formally recorded as verified.

## 관련 문서

- [빌드, 플래시 및 최초 기동](03_20260722_Build_Flash_and_First_Start.md)
- [하드웨어 검증 보고서](04_20260722_Hardware_Validation_Report.md)
- [최종 펌웨어 소스](../03_Board_Firmware/02_20260719_OctopusPro_PAROL6_Exp23__Final/README.md)
- [안전 경고 및 면책](../SAFETY_WARNING_AND_DISCLAIMER.md)
