# PAROL6 Octopus Pro Port

[English README](README_EN.md)

PAROL6 로봇암의 저수준 제어 보드 펌웨어를 BTT Octopus Pro V1.0 F446
보드에서 동작시키기 위한 비공식 실험용 포팅 작업 기록입니다.

## 상태

이 저장소는 아직 진행 중인 하드웨어/펌웨어 실험 기록입니다. 완성된 제어
보드 대체품이 아니며, 공식 PAROL6 control board의 drop-in 대안도 아닙니다.
Source Robotics와도 관련 없는 개인 실험 프로젝트입니다.

현재 초점:

- GPLv3로 공개된 PAROL6 low-level control-board firmware를 STM32F446ZE 기반
  BTT Octopus Pro V1.0 보드에 맞게 포팅합니다.
- PAROL6 프로토콜과 관절 순서를 유지하면서 step, dir, chip-select, enable,
  limit, E-stop 핀을 Octopus Pro에 맞게 다시 매핑합니다.
- TMC5160 드라이버, 24 V 센서, dry-contact 리미트 스위치, USB CDC, bootloader
  offset 관련 bring-up 과정을 문서화합니다.

## 원본 프로젝트

원본 PAROL6 프로젝트:

- 저장소: https://github.com/Source-Robotics/PAROL6-Desktop-robot-arm
- 이 작업에서 참고한 로컬 upstream snapshot:
  `896fa9fd793a076a0b3790b8a4a29a6f5bb12fcb`
- 라이선스: GPLv3

원본 PAROL6 전체 소스 snapshot은 이 저장소에 포함하지 않았습니다. upstream과
비교할 때는 원본 저장소를 별도로 clone해서 확인하는 것을 전제로 합니다.

## 저장소 구조

- `01_Research/` - 보드 적합성, 핀맵, 하드웨어 리스크 조사 노트
- `03_Board_Firmware/` - Octopus Pro용 펌웨어 포팅 작업
- `06_Docs/` - 배선, 핀 매핑, bring-up 순서 문서
- `000_INDEX.md` - 로컬 작업 폴더 인덱스

## 펌웨어 포팅

현재 펌웨어 draft 위치:

`03_Board_Firmware/01_20260606_OctopusProV1_Port__InProgress`

주요 가정:

- 보드: BTT Octopus Pro V1.0 F446 / STM32F446ZE
- 프레임워크: PlatformIO + Arduino STM32 core
- Bootloader offset: 32 KiB / `0x08008000`
- 클럭: 12 MHz HSE
- 모터 소켓: PAROL6 J1-J6를 Octopus MOTOR0-MOTOR5에 매핑
- 드라이버 제어: `TMCStepper` 기반 TMC5160-style SPI control
- USB serial: PAROL6 firmware의 baud rate 기대값에 맞춘 CDC serial

현재 핀맵과 bring-up 순서는
`06_Docs/01_20260606_OctopusProV1_Port_Notes.md`에 정리되어 있습니다.

## 안전 주의

이 프로젝트는 움직이는 로봇 하드웨어, 모터 드라이버, 24 V 배선, 펌웨어 기반
모션 제어를 다룹니다. 현재 draft firmware가 실제 하드웨어에서 안전하다고
가정하면 안 됩니다.

중요한 기본 원칙:

- PAROL6의 24 V inductive sensor를 Octopus endstop pin에 직접 연결하지
  마세요.
- optocoupler module 같은 24 V to 3.3 V 인터페이스를 사용하고, homing 전에
  polarity를 반드시 확인하세요.
- firmware의 E-stop 상태 보고는 telemetry로만 취급하세요. 실제 E-stop은 MCU
  코드와 무관하게 motor power를 차단하거나 driver를 비활성화하는 하드웨어
  안전 경로가 필요합니다.
- 가능하면 motor power를 분리한 상태에서 한 subsystem씩 테스트하세요.

이 저장소의 하드웨어 노트나 펌웨어를 사용하기 전에
`SAFETY_WARNING_AND_DISCLAIMER.md`를 읽어야 합니다.

## 라이선스

이 저장소는 GPLv3 PAROL6 source code를 기반으로 한 derivative firmware 작업을
포함하므로 GPLv3로 배포합니다. Third-party library는 각 디렉터리에 포함된
라이선스 고지를 따릅니다.
