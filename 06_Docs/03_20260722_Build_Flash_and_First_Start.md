# PAROL6 Octopus Pro 빌드, 플래시 및 최초 기동

이 절차는 **BTT Octopus Pro V1.0/V1.0.1 F446**와 이 저장소의 Exp23
최종 소스를 대상으로 합니다. 실제 검증 보드는 V1.0.1 F446입니다.

> 로봇암은 갑자기 움직이거나 중력으로 내려올 수 있습니다. 작업 공간을
> 비우고, 팔을 지지하며, STOP7 버튼과 별도의 24 V 직접 차단 수단을 즉시
> 사용할 수 있는 상태에서 진행하세요.

## 1. 대상 확인

다음 조건이 모두 맞아야 합니다.

- 보드 실크/MCU가 Octopus Pro F446 계열임
- MCU가 `STM32F446ZET6`임
- BTT 32 KiB SD bootloader가 유지되어 있음
- 애플리케이션 시작 주소가 `0x08008000`임
- 사용 드라이버가 24 V에서 사용 가능한 TMC5160T임
- [최종 배선표](02_20260722_Final_Wiring_and_Pin_Map.md)에 따라 MOTOR2와
  MOTOR7을 사용하지 않음

H723용 firmware와 혼용하지 마세요. DFU나 ST-Link로 주소를 잘못 지정하면
BTT bootloader를 덮어쓸 수 있습니다.

## 2. 릴리스 바이너리 확인

검증 대상으로 고정한 Exp23 바이너리 정보는 다음과 같습니다.

```text
file: firmware.bin
size: 50156 bytes
SHA-256:
66471679470987c078078b68e6dd05ace0703d8d5a25a77b6efaf81afdf7fee8
application address: 0x08008000
```

macOS에서는 다음 명령으로 확인할 수 있습니다.

```bash
shasum -a 256 firmware.bin
wc -c firmware.bin
```

Linux에서는 `sha256sum firmware.bin`을 사용할 수 있습니다. 해시가 다르면
그 파일을 이 문서의 검증된 바이너리로 간주하지 마세요.

## 3. 소스에서 빌드

최종 공개 소스 위치:

```text
03_Board_Firmware/02_20260719_OctopusPro_PAROL6_Exp23__Final
```

기준 도구:

- PlatformIO Core `6.1.19`
- ST STM32 platform `19.6.0`
- Arduino STM32 framework `2.12.0`
- GNU Arm Embedded GCC `12.3.1`

저장소 루트에서 다음과 같이 빌드합니다.

```bash
cd 03_Board_Firmware/02_20260719_OctopusPro_PAROL6_Exp23__Final
pio run -e btt_octopus_pro_v1_0_f446ze_literal_upstream
```

출력:

```text
.pio/build/btt_octopus_pro_v1_0_f446ze_literal_upstream/firmware.bin
```

설정은 12 MHz HSE, 180 MHz core, USB CDC와 32 KiB bootloader offset을
사용합니다. 다른 PlatformIO/toolchain 버전으로 빌드한 결과는 기능이 같아도
byte-for-byte 해시가 달라질 수 있으므로 별도 빌드로 기록하세요.

## 4. microSD 플래시

1. 보드의 24 V 전원과 USB를 모두 분리합니다.
2. FAT32로 포맷된 microSD 카드의 루트에 `firmware.bin` 하나만 넣습니다.
3. 카드를 안전하게 꺼내 Octopus에 삽입합니다.
4. STOP7 버튼을 누른 상태, 즉 NC loop가 열린 상태로 둡니다.
5. 보드 전원을 켜고 SD update가 끝날 시간을 충분히 기다립니다.
6. 보드 전원을 끈 뒤 microSD를 꺼냅니다.
7. `firmware.bin`이 `FIRMWARE.CUR`로 바뀌었는지 확인합니다.

`FIRMWARE.CUR`로 바뀌지 않았다면 motion 시험으로 넘어가지 마세요. 카드
포맷, 파일 이름, 보드 리비전과 bootloader 상태부터 확인하세요.

## 5. 최초 무구동 확인

모터 전원은 아직 넣지 않습니다.

1. 모터 케이블, 센서 인터페이스와 드라이버 방향을 육안 검사합니다.
2. USB만 연결해 USB CDC가 인식되는지 확인합니다.
3. 제어 프로그램은 하나만 실행합니다. Waldo와 다른 Commander가 동시에
   같은 serial port를 열지 않게 합니다.
4. STOP7을 누르면 stop 상태, 해제하면 물리 접점이 닫히는지 확인합니다.
5. J1–J6 센서를 한 개씩 작동시켜 [최종 배선표](02_20260722_Final_Wiring_and_Pin_Map.md)의
   극성과 일치하는지 확인합니다.
6. 24 V 근접센서의 MCU 측 출력이 3.3 V 안전 범위인지 멀티미터로 확인합니다.

USB CDC는 baud 설정과 무관하게 전송되지만, firmware와 host 설정은 원본
PAROL6 값인 `3,000,000`을 유지합니다. Exp23의 USB manufacturer string은
provenance 보존 때문에 내부적으로 `PAROL6_OCTOPUS_EXP22`로 남아 있습니다.

## 6. 24 V 최초 기동

1. USB와 전원을 다시 분리합니다.
2. 모든 SPI/전압 점퍼, 24 V 극성, 모터 코일 쌍을 재확인합니다.
3. 로봇 관절을 지지하고 가동 범위를 비웁니다.
4. STOP7을 누른 상태로 MB 전원과 motor power 24 V를 인가합니다.
5. 이상 발열, 냄새, 소리 또는 드라이버 fault가 없는지 확인합니다.
6. USB를 연결하고 Waldo 또는 호환 Commander를 실행합니다.
7. STOP7을 해제합니다.
8. UI에서 Reset을 수행한 뒤 별도로 Enable합니다.
9. 정지 토크가 예상대로 생기는지 확인합니다.
10. 저속·소각도 단축 동작으로 각 축 방향을 확인합니다.
11. 모든 축과 센서가 맞을 때만 전체 HOME을 실행합니다.

STOP7이 열린 채 부팅하면 드라이버는 enable되지 않습니다. 버튼을 해제한
것만으로는 복구되지 않으며 반드시 Reset 후 별도 Enable이 필요합니다.

## 7. 전체 HOME 전 확인

- J1–J6의 limit 입력을 각각 직접 확인함
- 각 관절의 sensor 방향과 반대 방향을 저속으로 확인함
- 케이블이 관절에 감기거나 당겨지지 않음
- J2/J3처럼 중력 영향을 받는 축을 지지할 준비가 됨
- STOP7과 24 V 직접 차단 장치를 즉시 사용할 수 있음
- 다른 serial controller가 실행 중이지 않음

원본 HOME 순서는 J1–J3, J4, J6, J5이며 각 sensor를 첫 감지, 이탈,
재감지한 뒤 원본 offset/standby 위치로 이동합니다. HOME 중 STOP7이
작동하면 HOME은 취소됩니다. 자동 재개하지 말고 원인을 확인한 뒤 Reset과
Enable부터 다시 수행하세요.

## 8. FAN0 밸브 최초 시험

실제 밸브 작동은 아직 정식 검증 완료 항목이 아닙니다. 전체 로봇 시험과
분리해서 진행하세요.

1. 24 V와 USB를 모두 분리합니다.
2. FAN0 jumper를 `VIN/V_FUSED`로 확인합니다.
3. 코일을 연결하지 않은 채 보드에 MB 24 V와 USB만 연결합니다.
4. pneumatic port 1 `close`에서 FAN0 양단 약 0 V인지 측정합니다.
5. `open`에서 약 24 V인지 측정합니다.
6. 다시 `close`에서 약 0 V인지 확인합니다.
7. 모든 전원을 끈 뒤 코일 `+/-`를 FAN0 `+/-`에 연결합니다.
8. 공압을 공급하지 않은 상태에서 `close → open → close` 한 번으로 밸브
   클릭과 발열을 확인합니다.
9. 이상이 없을 때만 낮은 공압부터 시험합니다.

FAN0은 low-side MOSFET 출력입니다. 코일 한쪽을 임의로 외부 GND에 우회하지
마세요. STOP7과 USB 단절은 밸브를 자동으로 OFF시키지 않습니다.

## 9. 제어 프로그램

- Waldo Web Commander: 이 프로젝트의 주 실기 제어 환경
- PAROL6 Python API: 원본 56/60-byte protocol과 호환
- 공식 PAROL Commander: 원본 protocol 호환
- Better Command: 기본 통신은 사용할 수 있으나 Action Queue 프로그램
  실행기는 이 프로젝트에서 완전 검증되지 않음

각 제어 프로그램은 한 번에 하나만 serial port를 소유해야 합니다.

## English summary

Use this procedure only with an Octopus Pro F446 board. Verify the release
binary SHA-256, flash `firmware.bin` from the root of a FAT32 microSD card, and
confirm that the bootloader renamed it to `FIRMWARE.CUR`. Perform USB and input
checks without motor power first. For powered commissioning, boot with STOP7
open, support the arm, then close STOP7, Reset, and separately Enable. Verify
each axis and sensor before running the original full HOME sequence. FAN0
pneumatic-valve actuation remains a separate, not-formally-verified test.

## 관련 문서

- [최종 배선 및 핀맵](02_20260722_Final_Wiring_and_Pin_Map.md)
- [하드웨어 검증 보고서](04_20260722_Hardware_Validation_Report.md)
- [최종 펌웨어 README](../03_Board_Firmware/02_20260719_OctopusPro_PAROL6_Exp23__Final/README.md)
- [안전 경고 및 면책](../SAFETY_WARNING_AND_DISCLAIMER.md)
