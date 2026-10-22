# PAROL6 Octopus Pro Port

[English](README_EN.md) | [안전 경고](SAFETY_WARNING_AND_DISCLAIMER.md) | [배선표](06_Docs/02_20260722_Final_Wiring_and_Pin_Map.md) | [펌웨어 바이너리](07_Output/01_20260722_PAROL6_OctopusPro_v1.0.0__Final/firmware.bin)

PAROL6 로봇암의 원본 저수준 제어 펌웨어를 **BTT Octopus Pro V1.0/V1.0.1 F446** 보드에서 실행하기 위한 비공식 포트입니다. PAROL6의 56/60바이트 통신 프로토콜, 관절 프로파일, 호밍 상태 머신을 최대한 그대로 유지하고, 보드 변경에 필요한 핀, 드라이버, USB, STOP7, 보조 출력 계층만 이식했습니다.

> 이 프로젝트는 Source Robotics 또는 BIGTREETECH의 공식 제품이 아니며, 인증된 안전 제어기가 아닙니다. 움직이는 로봇과 24V 전원을 다루기 전에 [안전 경고](SAFETY_WARNING_AND_DISCLAIMER.md)를 반드시 읽으세요.

## v1.0.0 상태

- 실제 하드웨어: **BTT Octopus Pro V1.0.1 F446 / STM32F446ZET6**
- 모터 드라이버: **BIGTREETECH TMC5160T**, SPI, 32 microsteps
- 전원: 로봇과 모터 구동부 24V
- 검증 완료: SD 펌웨어 플래시, USB CDC, 6축 구동, 6개 리미트 센서, STOP7 정지/토크 유지/복구, 최초 및 반복 전체 HOME, Waldo Web Commander, 샘플 dance, 4개 냉각팬
- 구현 후 정적 검증: FAN0의 24V 단일 솔레노이드 출력
- 아직 기록되지 않은 실물 검증: 실제 공압 솔레노이드 코일 및 공압 전환

최종 소스와 바이너리는 다음 위치에 있습니다.

- [최종 펌웨어 소스](03_Board_Firmware/02_20260719_OctopusPro_PAROL6_Exp23__Final/)
- [v1.0.0 펌웨어와 릴리스 노트](07_Output/01_20260722_PAROL6_OctopusPro_v1.0.0__Final/)
- [Waldo/PAROL6 Python API 호환 패치](02_Host_Integration/01_20260718_Waldo_STOP7_Torque_Hold_Patches__Final/)
- [Octopus Pro 보드 케이스 STL](04_3D_Printing/01_Octopus_Pro_Board_Enclosure/)

## 지원 범위

이 릴리스는 **Octopus Pro F446 계열에만** 맞춰져 있습니다. H723 등 다른 MCU 변형에는 그대로 사용하면 안 됩니다.

| 항목 | 설정 |
|---|---|
| MCU | STM32F446ZET6, 180MHz |
| 외부 클럭 | 12MHz HSE |
| 부트로더 오프셋 | 32KiB, `0x08008000` |
| USB | CDC, VID:PID `0483:5740`, nominal 3,000,000 baud |
| 모터 전원 | 24V로 실물 검증 |
| SPI | SCK `PA5`, MISO `PA6`, MOSI `PA7` |

컴파일된 USB manufacturer 문자열은 호환성 검증을 유지하기 위해 `PAROL6_OCTOPUS_EXP22`로 남아 있습니다. v1.0.0 동작에는 영향이 없는 표시용 문자열입니다.

## 최종 관절과 센서 매핑

| 관절 | 모터 소켓 | STEP | DIR | CS | EN | 리미트 | 핀 | 트리거 |
|---|---|---|---|---|---|---|---|---|
| J1 | MOTOR6 | `PE2` | `PE3` | `PE1` | `PD4` | STOP3 | `PG11` | LOW |
| J2 | MOTOR5 | `PC13` | `PF0` | `PE4` | `PF1` | STOP2 | `PG10` | HIGH |
| J3 | MOTOR4 | `PF9` | `PF10` | `PF2` | `PG2` | STOP1 | `PG9` | HIGH |
| J4 | MOTOR3 | `PG4` | `PC1` | `PC7` | `PA0` | STOP4 | `PG12` | LOW |
| J5 | MOTOR1 | `PG0` | `PG1` | `PD11` | `PF15` | STOP0 | `PG6` | HIGH |
| J6 | MOTOR0 | `PF13` | `PF12` | `PC4` | `PF14` | STOP5 | `PG13` | LOW |

`MOTOR2`는 Octopus의 병렬 Z 출력 구조 때문에 사용하지 않으며 `MOTOR7`도 비워 둡니다. 상세 배선, 센서 종류, 점퍼와 팬/밸브 연결은 [최종 배선 및 핀맵](06_Docs/02_20260722_Final_Wiring_and_Pin_Map.md)을 따르세요.

## STOP7 동작

STOP7은 `PG15`에 연결한 **NC(normally closed)** 접점입니다.

- 정상/배선 연결: LOW
- 버튼 누름 또는 배선 단선: HIGH
- HIGH가 되면 STEP 출력을 즉시 차단하고 진행 중인 이동/HOME을 취소합니다.
- 이미 활성화된 드라이버의 EN은 유지하므로 중력축의 정지 토크가 남습니다.
- 버튼을 해제해도 자동 재개하지 않습니다. 패치된 호스트에서 Reset 후 별도로 Resume/Enable 해야 합니다.
- 부팅할 때 STOP7이 열려 있으면 드라이버를 활성화하지 않으므로 정지 토크도 없습니다.

이 기능은 로봇이 전원 차단 시 처지는 상황을 고려한 **소프트웨어 정지/토크 유지 기능**입니다. 안전 등급 하드 E-stop이 아니므로, 별도의 즉시 접근 가능한 24V 전원 차단 수단을 함께 사용해야 합니다.

## 빠른 시작

1. [빌드, 플래시, 첫 구동 문서](06_Docs/03_20260722_Build_Flash_and_First_Start.md)와 안전 경고를 먼저 읽습니다.
2. 보드가 Octopus Pro **F446**인지, 전원 극성, 드라이버 전압 선택 점퍼, SPI 점퍼를 전원이 꺼진 상태에서 확인합니다.
3. 24V 근접센서는 보드 입력에 직접 연결하지 말고 3.3V 로직으로 절연/레벨 변환합니다.
4. [`firmware.bin`](07_Output/01_20260722_PAROL6_OctopusPro_v1.0.0__Final/firmware.bin)을 FAT32 microSD 루트에 복사합니다.
5. 전원이 꺼진 보드에 카드를 넣고 전원을 켭니다. 성공하면 파일명이 `FIRMWARE.CUR`로 바뀝니다.
6. STOP7과 모든 리미트 입력을 먼저 확인한 뒤, 낮은 위험 조건에서 한 축씩 방향을 검증합니다.
7. Waldo를 사용할 경우 [호스트 호환 패치](02_Host_Integration/01_20260718_Waldo_STOP7_Torque_Hold_Patches__Final/)를 적용해야 STOP7의 토크 유지/수동 복구 의미가 보존됩니다.

배포 바이너리 SHA-256:

```text
66471679470987c078078b68e6dd05ace0703d8d5a25a77b6efaf81afdf7fee8  firmware.bin
```

## 소스 빌드

PlatformIO Core 6.1.19와 `ststm32@19.6.0`으로 검증했습니다.

```bash
cd 03_Board_Firmware/02_20260719_OctopusPro_PAROL6_Exp23__Final
pio run
```

기본 환경은 `btt_octopus_pro_v1_0_f446ze_literal_upstream`입니다. 자세한 빌드 및 테스트 방법은 [펌웨어 README](03_Board_Firmware/02_20260719_OctopusPro_PAROL6_Exp23__Final/README.md)에 있습니다.

## 보조 출력

- FAN4/`PD14`, FAN5/`PD15`: 부팅 후 ON
- FAN6, FAN7: `V_FAN`에 하드웨어 직결되어 항상 ON
- 실제 구성: 12V 4040 팬 4개. 팬 전압 선택 레일을 반드시 12V로 설정
- FAN0/`PA8` = PAROL6 `OUTPUT1`: DC24V 단일 솔레노이드용, LOW=OFF/닫힘, HIGH=ON/열림

FAN0을 24V로 사용할 때는 해당 포트의 전압 선택 점퍼를 `VIN/V_FUSED`에 두고, 연결 전 코일 정격, 극성, 보호 회로를 확인하세요. 현재 릴리스에는 통신/핀/부팅 OFF 동작 검증은 포함되지만 실제 공압 밸브의 물리 시험 기록은 포함되지 않습니다.

## 저장소 구조

- `02_Host_Integration/`: STOP7 복구 의미를 보존하는 호스트 패치
- `03_Board_Firmware/02_...__Final/`: 재현 가능한 최종 펌웨어 소스
- `04_3D_Printing/`: Octopus Pro 및 4×4040 팬용 케이스 STL
- `06_Docs/`: 최종 배선, 플래시, 검증 문서
- `07_Output/01_...v1.0.0__Final/`: 배포 바이너리, 체크섬, 릴리스 노트

번호 `01`의 초기 펌웨어와 2026-06-06 문서는 개발 초안이며 최종 배선에 사용하면 안 됩니다.

## 원본과 라이선스

이 포트는 [PCrnjak/PAROL6-Desktop-robot-arm](https://github.com/PCrnjak/PAROL6-Desktop-robot-arm)의 GPLv3 펌웨어를 commit `896fa9fd793a076a0b3790b8a4a29a6f5bb12fcb`에서 고정해 작업했습니다. 원본 모션 프로토콜과 HOME 함수의 동일성은 자동 테스트로 확인합니다.

저장소의 파생 펌웨어와 프로젝트 자료는 루트 [GPLv3 LICENSE](LICENSE)에 따라 배포합니다. 포함 라이브러리의 저작권과 개별 조건은 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) 및 각 라이브러리의 LICENSE를 따릅니다.

## 검증 기록과 알려진 한계

정확한 시험 범위는 [하드웨어 검증 보고서](06_Docs/04_20260722_Hardware_Validation_Report.md)에 기록합니다.

- 이 프로젝트는 단일 제작품에서 검증된 포트이며 범용 완제품 컨트롤러가 아닙니다.
- STOP7, USB 통신, 센서, 호스트 앱은 안전 인증을 받지 않았습니다.
- USB가 끊기거나 STOP7이 걸려도 FAN0 출력에는 watchdog이 없으며 마지막 상태를 유지할 수 있습니다.
- Better Command의 모든 프로그램 실행 기능은 이 릴리스에서 완전 호환으로 검증하지 않았습니다.
- 자동 HOME 전에는 반드시 실제 기구 조립, 센서 위치, 움직임 여유를 확인하세요.
