# PAROL6 BTT Octopus Pro Board Enclosure

[English](#english)

## 한국어

PAROL6의 제어 보드를 **BTT Octopus Pro V1.0.1 F446**로 교체하기 위해
제작한 2분할 전장 케이스입니다. 메인 케이스와 상단 커버로 구성되며,
냉각용 **40 x 40 mm(4040) DC 12 V 팬 4개**를 장착하도록 설계했습니다.

### 포함 파일

| 파일 | 설명 | 메시 외형 치수 (mm) | 삼각형 | SHA-256 |
| --- | --- | ---: | ---: | --- |
| `PAROL6_BTT_OctopusPro_4x4040Fan_Case_Main_v3.stl` | 메인보드 및 팬 장착용 메인 케이스 | 178.0 x 129.7 x 59.65 | 13,602 | `33eb5e03313dea2f16948e5e4e7ed93baed4119a777ea8b93b3c0ee54793e778` |
| `PAROL6_BTT_OctopusPro_4x4040Fan_Case_TopCover_v2.stl` | 상단 커버 | 176.1 x 117.9 x 10.0 | 7,780 | `dc42713f817109b0b7e956fc3eacbeb8086a5a1145b4c49f672848f95d9613ef` |

두 STL은 mm 단위의 바이너리 STL입니다. 공개 전 검사에서 열린 경계,
비다양체 엣지 및 퇴화 삼각형이 발견되지 않았습니다.

### 호환성과 출력

- 실제 장착 검증 대상은 이 프로젝트에서 사용하는 BTT Octopus Pro
  V1.0.1 F446 보드입니다.
- 다른 Octopus Pro 리비전은 포트 위치와 외형을 비교한 후 사용하세요.
- 슬라이서에서 단위가 mm인지 확인하고, 사용 프린터와 재료에 맞게 출력
  방향과 서포트를 검토하세요.

### 12 V 팬 주의사항

이 STL은 기계 장착 구조만 정의합니다. 12 V 팬을 연결하기 전에 Octopus Pro의
팬 전압 선택 점퍼가 **12 V 레일**로 설정되어 있는지 확인하고 극성을 다시
점검하세요. 24 V VIN이 선택된 출력에 12 V 팬을 연결하면 팬이 손상될 수
있습니다.

## English

This two-piece electronics enclosure was designed for a PAROL6 converted to a
**BTT Octopus Pro V1.0.1 F446** controller. It consists of a main enclosure and
a top cover, with mounting provisions for **four 40 x 40 mm (4040) DC 12 V
fans**.

### Included files

| File | Description | Mesh bounds (mm) | Triangles |
| --- | --- | ---: | ---: |
| `PAROL6_BTT_OctopusPro_4x4040Fan_Case_Main_v3.stl` | Main board and fan enclosure | 178.0 x 129.7 x 59.65 | 13,602 |
| `PAROL6_BTT_OctopusPro_4x4040Fan_Case_TopCover_v2.stl` | Top cover | 176.1 x 117.9 x 10.0 | 7,780 |

Both models are binary STL files in millimetres. A pre-publication mesh check
found no open boundary edges, non-manifold edges, or degenerate triangles.

The validated target is the BTT Octopus Pro V1.0.1 F446 used in this project.
Check the board outline and connector locations before using another hardware
revision. Confirm that the fan-voltage selector is set to the **12 V rail** and
verify polarity before connecting the fans; a 12 V fan can be damaged if its
output is configured for 24 V VIN.

## License

These models are distributed under the repository's root license unless a
model-specific notice is added later.
