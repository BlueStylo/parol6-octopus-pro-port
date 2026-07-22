# PAROL6 Octopus Pro 하드웨어 검증 보고서

## 판정

PAROL6 원본 저수준 제어 펌웨어를 BTT Octopus Pro V1.0.1 F446에 옮긴
Exp23 포트는 실제 로봇에서 6축 구동, 센서, STOP7 torque-hold, 최초 HOME,
반복 HOME, Waldo 제어와 샘플 동작까지 확인했습니다.

이 결과는 **한 대의 자작 PAROL6와 한 장의 Octopus Pro V1.0.1 F446에서
제작자가 관찰한 기능 검증**입니다. 인증 시험, 기능 안전 평가, 수명 시험,
EMC 시험 또는 다른 조립체에 대한 보증이 아닙니다.

## 검증 대상

| Item | Value |
| --- | --- |
| Board | BTT Octopus Pro V1.0.1 F446 |
| MCU | STM32F446ZET6 |
| Motor drivers | BTT TMC5160T × 6, SPI |
| Supply used | DC 24 V main and motor rails |
| Firmware | Exp23 literal-upstream minimal port + FAN0 OUTPUT1 mapping |
| Firmware size | 50,156 bytes |
| Firmware SHA-256 | `66471679470987c078078b68e6dd05ace0703d8d5a25a77b6efaf81afdf7fee8` |
| Application address | `0x08008000` |
| Primary controller | Waldo Web Commander / PAROL6 Python API |

보드에서 firmware 내용을 다시 읽어 SHA-256을 비교하는 read-back 감사는
수행하지 않았습니다. 따라서 위 해시는 공개 release artifact와 clean build의
동일성을 나타내며, 보드 flash의 독립적인 read-back 증명은 아닙니다.

## 원본 provenance

```text
repository:
https://github.com/PCrnjak/PAROL6-Desktop-robot-arm.git

pinned commit:
896fa9fd793a076a0b3790b8a4a29a6f5bb12fcb

pinned firmware tree:
2bac41ef589ae8e253384a977c83b8526726becc
```

Exp23은 원본 protocol, motor profile, `reset_homing()`과 `home_all()`을
보존하고 Octopus의 pin, clock, 개별 EN/CS, 지원하지 않는 주변회로와 STOP7
adapter만 분리했습니다. Exp22에서 Exp23으로 바뀐 의도된 기능 차이는
`OUTPUT1: PE14 → FAN0/PA8`이며 두 clean binary의 차이는 정확히 5 bytes로
감사됐습니다.

## 오프라인 검증

| Test | Result |
| --- | --- |
| Python static/protocol/pneumatic regression | PASS — 21 tests |
| Native Octopus GPIO/STOP7 state-machine tests | PASS |
| Production `reset_homing()` extraction and native regression | PASS |
| Production `home_all()` first/repeat HOME native regression | PASS |
| Pinned-upstream source identity checks | PASS |
| Clean STM32F446ZE build | PASS |
| Vector, entry point, size and SHA audit | PASS |
| Exp22→Exp23 binary delta audit | PASS — 5 bytes |

고정 함수 해시:

```text
reset_homing SHA-256:
2116c84242001755e564efd2f8cc052e53e73a8a47678bedc955b459d478606b

home_all SHA-256:
afdcc71b3a22710239f2fcfd84882687fd5b8dbc7b45ce1dfe155affd2db9c1f
```

빌드 환경:

```text
PlatformIO Core: 6.1.19
ST STM32 platform: 19.6.0
Arduino STM32 framework: 2.12.0
GNU Arm Embedded GCC: 12.3.1
RAM: 8188 / 131072 bytes
Flash sections: 49640 / 524288 bytes
```

## 실제 하드웨어에서 확인한 항목

| Item | Result | Evidence level |
| --- | --- | --- |
| microSD bootloader update and `FIRMWARE.CUR` workflow | PASS | 제작 과정 관찰 |
| USB CDC enumeration and packet communication | PASS | 실기 관찰 |
| Six TMC5160T driver slots and holding torque | PASS | 실기 관찰 |
| J1–J6 one-axis direction and motion | PASS | 축별 실기 관찰 |
| J1–J6 limit sensor polarity and stop direction | PASS | 축별 실기 관찰 |
| MOTOR2 bypass / final six-socket remap | PASS | 실기 관찰 |
| STOP7 stop while J6 was moving | PASS | 즉시 정지 관찰 |
| STOP7 holding torque retained | PASS | 정지 후 수동 확인 |
| STOP7 release without automatic resume | PASS | 실기 관찰 |
| Reset followed by separate Enable recovery | PASS | 실기 관찰 |
| First full six-axis HOME | PASS | 실기 관찰 |
| Immediate repeated full HOME | PASS | 실기 관찰 |
| Waldo Web Commander motion and HOME | PASS | 실기 관찰 |
| Sample dance/program motion | PASS | 실기 및 영상 관찰 |
| Four 12 V 4040 cooling fans | PASS | 네 팬 동작 직접 확인 |
| FAN0 no-load 0/24 V measurement | NOT FORMALLY RECORDED | 확인 기록 없음 |
| FAN0 DC 24 V solenoid actuation | NOT FORMALLY VERIFIED | 확인 기록 없음 |
| Pneumatic gripper under pressure/load | NOT FORMALLY VERIFIED | 확인 기록 없음 |

실기 과정에서 조립 설명서의 센서 위치 오류와 기계 조립 문제도 발견되어
수정됐습니다. 최종 HOME 결과는 이 기계 수정 후의 상태를 기준으로 합니다.

## 검증된 최종 매핑

| Joint | Motor | Limit | Active |
| --- | --- | --- | --- |
| J1 | MOTOR6 | STOP3 / PG11 | LOW |
| J2 | MOTOR5 | STOP2 / PG10 | HIGH |
| J3 | MOTOR4 | STOP1 / PG9 | HIGH |
| J4 | MOTOR3 | STOP4 / PG12 | LOW |
| J5 | MOTOR1 | STOP0 / PG6 | HIGH |
| J6 | MOTOR0 | STOP5 / PG13 | LOW |

MOTOR2와 MOTOR7은 사용하지 않습니다. 상세 STEP/DIR/CS/EN pin은
[최종 배선 및 핀맵](02_20260722_Final_Wiring_and_Pin_Map.md)에 있습니다.

## STOP7 검증 범위

STOP7 `PG15`는 NC loop가 닫혔을 때 LOW, 버튼 누름 또는 단선 시 HIGH입니다.
실기에서는 움직이던 J6가 정지하고 이미 활성화된 모터의 정지 토크가 남는
것을 확인했습니다. 접점 해제만으로 재개하지 않았고 Reset 후 별도 Enable로
복구했습니다.

이 결과는 firmware STEP gating 동작을 확인한 것이며 IEC 60204-1, ISO
13849 또는 다른 기능 안전 표준에 따른 검증이 아닙니다. 24 V 직접 차단
수단을 별도로 유지해야 합니다.

## 알려진 제한과 미검증 범위

- 실제 검증은 Octopus Pro V1.0.1 F446 한 대에서만 수행했습니다.
- H723과 다른 보드 revision은 지원하지 않습니다.
- FAN0 valve 경로는 소스와 protocol 회귀만 통과했으며 실제 코일/공압 시험
  성공은 정식 기록되지 않았습니다.
- CAN transceiver와 원본 CAN gripper hardware는 no-op adapter이며 미지원입니다.
- 원본 PB0 bus-voltage ADC와 soft power latch는 Octopus에 없어 링크하지
  않습니다.
- 원본 serial flash 기능은 현재 build에 링크하지 않습니다.
- OUTPUT2 `PE15`는 logic-level spare이며 power output으로 검증하지 않았습니다.
- STOP7 또는 USB 단절 시 FAN0은 마지막 상태를 유지합니다.
- UI의 OUTPUT1 표시는 feedback sensor가 아니라 명령 echo입니다.
- 전원공급장치 용량, 장시간 열 안정성, payload별 전류 여유와 연속 운전
  수명은 별도 검증 대상입니다.
- 공식 PAROL Commander와 packet 호환성은 유지하지만 이 검증의 주 실기
  환경은 Waldo였습니다. Better Command의 Action Queue 실행기는 완전 검증
  범위가 아닙니다.
- USB descriptor의 manufacturer string은 Exp23에서도 provenance 보존을 위해
  `PAROL6_OCTOPUS_EXP22`로 남아 있습니다.

## 결론

이 포트는 개인 제작물에서 실제 6축 HOME과 프로그램 동작까지 성공한
**hardware-validated experimental port**로 공개할 수 있습니다. 다만 공식
PAROL6 control board의 인증된 drop-in replacement 또는 안전 controller로
표현해서는 안 됩니다. FAN0 pneumatic 기능은 구현 완료, 정식 물리 검증
대기 상태로 공개해야 합니다.

## English summary

Exp23 was observed running on one BTT Octopus Pro V1.0.1 F446 PAROL6 build.
Six-axis motion, all limit inputs, STOP7 step blocking with holding torque,
first and repeated full homing, Waldo control, a sample dance program, and four
12 V cooling fans passed operator-observed testing. Offline protocol, STOP7,
upstream-home, binary and clean-build regressions also passed. This is not a
certified safety validation or a universal drop-in replacement. FAN0 software
and protocol routing are verified, but physical 24 V solenoid/pneumatic
actuation has not been formally recorded as verified.

## 관련 문서

- [최종 배선 및 핀맵](02_20260722_Final_Wiring_and_Pin_Map.md)
- [빌드, 플래시 및 최초 기동](03_20260722_Build_Flash_and_First_Start.md)
- [최종 firmware validation](../03_Board_Firmware/02_20260719_OctopusPro_PAROL6_Exp23__Final/VALIDATION.md)
- [안전 경고 및 면책](../SAFETY_WARNING_AND_DISCLAIMER.md)
