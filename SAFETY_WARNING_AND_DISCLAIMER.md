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
- Use a real hardware E-stop path that removes motor power or disables drivers
  independently of firmware.
- Do not run automatic homing or motion until sensor polarity and joint
  direction are confirmed at low power.

This is not an official Source Robotics project and is not a certified safety
system.
