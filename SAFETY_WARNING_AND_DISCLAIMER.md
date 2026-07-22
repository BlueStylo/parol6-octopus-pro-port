# Safety Warning And Disclaimer

This repository documents an unofficial experimental firmware port for robotic
hardware. It may contain errors, incomplete pin mappings, unsafe assumptions,
or untested code paths.

Robotic arms, stepper motors, motor drivers, power supplies, 24 V sensors, and
software-controlled motion can cause unexpected movement, pinch points, crush
hazards, electrical shock, property damage, serious injury, or death.

By using any information, source code, firmware, wiring notes, or build
instructions from this repository, you accept full responsibility for safe
assembly, testing, operation, and risk assessment.

No warranty is provided. The repository is provided as-is, without any guarantee
of correctness, reliability, fitness for a particular purpose, or safety.

Minimum safety expectations:

- Use current-limited bench testing where possible.
- Keep motor power disconnected during firmware upload and pin verification.
- Do not connect 24 V sensors directly to MCU-level inputs.
- Verify every driver enable, step, direction, chip-select, limit, and E-stop
  signal before connecting the robot drivetrain.
- STOP7 in this port is a software halt that intentionally retains holding
  torque. It is not a certified hard E-stop and cannot replace an independent,
  immediately accessible 24 V power cutoff.
- Removing motor power can allow gravity-loaded joints to fall. Support the arm
  and assess that hazard before using the hard power cutoff.
- Do not run automatic homing or motion until sensor polarity and joint
  direction are confirmed at low power.
- Do not leave the robot unattended while energized.

The pneumatic FAN0 output has no safety watchdog. A communications disconnect
or STOP7 event may leave its last commanded state active; design the pneumatic
system so loss of control cannot create a hazardous state.

This is not an official Source Robotics project and is not a certified safety
system.
