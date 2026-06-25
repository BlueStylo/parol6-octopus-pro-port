# PAROL6 Robot Arm

This is the working root for PAROL6 robot-arm experiments, including firmware adaptation for 3D-printer-style control boards.

## Folders

- `01_Research`: reference notes, board pinout research, BOM/build findings
- `02_Source`: upstream source code, firmware snapshots, hardware configs
- `03_Board_Firmware`: board-specific firmware edits and build outputs
- `04_3D_Printing`: slicer profiles, print notes, printed-part experiments
- `05_Experiments`: dated experiment folders such as `01_20260606_Board_Port__InProgress`
- `06_Docs`: manuals, wiring notes, setup docs, decision records
- `07_Output`: exported binaries, packaged configs, final artifacts
- `90_Archive`: old or replaced attempts kept for reference
- `99_Temp`: scratch files and short-lived tests

## Notes

- Use dated subfolders in `05_Experiments` for each concrete experiment.
- Keep board firmware changes separate from print-setting experiments unless a test explicitly needs both.
- Upstream PAROL6 repository downloaded to `02_Source/01_PAROL6_Desktop_Robot_Arm__Upstream`; official firmware lives in `PAROL6 control board main software` and `PAROL6 control board test code`.
- Octopus Pro V1.0 F446 port draft lives in `03_Board_Firmware/01_20260606_OctopusProV1_Port__InProgress`; notes are in `06_Docs/01_20260606_OctopusProV1_Port_Notes.md`.
