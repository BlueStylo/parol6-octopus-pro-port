from __future__ import annotations

import json
import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def macros() -> dict[str, str]:
    text = (ROOT / "src/iodefs.h").read_text()
    return dict(
        re.findall(r"^#define\s+([A-Z][A-Z0-9_]*)\s+([A-Z][A-Z0-9_]*)", text, re.M)
    )


class HardwareContractTest(unittest.TestCase):
    def test_joint_to_octopus_pin_map(self) -> None:
        value = macros()
        expected = {
            "PUL1": "PE2",
            "DIR1": "PE3",
            "SELECT1": "PE1",
            "ENABLE1": "PD4",
            "PUL6": "PC13",
            "DIR6": "PF0",
            "SELECT6": "PE4",
            "ENABLE6": "PF1",
            "PUL5": "PF9",
            "DIR5": "PF10",
            "SELECT5": "PF2",
            "ENABLE5": "PG2",
            "PUL4": "PG4",
            "DIR4": "PC1",
            "SELECT4": "PC7",
            "ENABLE4": "PA0",
            "PUL2": "PG0",
            "DIR2": "PG1",
            "SELECT2": "PD11",
            "ENABLE2": "PF15",
            "PUL3": "PF13",
            "DIR3": "PF12",
            "SELECT3": "PC4",
            "ENABLE3": "PF14",
        }
        for name, pin in expected.items():
            self.assertEqual(pin, value[name], name)

    def test_limit_spi_stop_and_fan_map(self) -> None:
        value = macros()
        expected = {
            "LIMIT1": "PG13",
            "LIMIT2": "PG10",
            "LIMIT3": "PG9",
            "LIMIT4": "PG12",
            "LIMIT5": "PG6",
            "LIMIT6": "PG11",
            "ESTOP": "PG15",
            "SCK": "PA5",
            "MISO": "PA6",
            "MOSI": "PA7",
            "FLASH_SELECT": "PF8",
            "FAN0": "PA8",
            "FAN4": "PD14",
            "FAN5": "PD15",
        }
        for name, pin in expected.items():
            self.assertEqual(pin, value[name], name)

    def test_pneumatic_output1_uses_fan0_power_stage(self) -> None:
        value = macros()
        self.assertEqual("PA8", value["FAN0"])
        self.assertEqual("FAN0", value["OUTPUT1"])
        self.assertEqual("PE15", value["OUTPUT2"])

        main = (ROOT / "src/main.cpp").read_text()
        adapter = (ROOT / "src/octopus_board.cpp").read_text()
        self.assertIn(
            "digitalWrite(OUTPUT1, PAROL6.commanded_OUT1);",
            main,
        )
        self.assertIn("Preload_Output(OUTPUT1, LOW);", adapter)

    def test_pneumatic_output_preserves_original_hold_last_policy(self) -> None:
        main = (ROOT / "src/main.cpp").read_text()
        adapter = (ROOT / "src/octopus_board.cpp").read_text()

        # The board adapter owns only the boot-safe OFF preload. STOP7 gates
        # motion STEP pins and deliberately does not rewrite pneumatic state.
        self.assertEqual(1, adapter.count("Preload_Output(OUTPUT1, LOW);"))
        self.assertNotIn("digitalWrite(OUTPUT1", adapter)

        # The literal upstream frame handler is the sole runtime writer. With
        # no newly parsed host frame, the MOSFET therefore holds its last state.
        self.assertEqual(
            1,
            main.count(
                "digitalWrite(OUTPUT1, PAROL6.commanded_OUT1);"
            ),
        )

    def test_upstream_home_signs_keep_validated_physical_dir_levels(self) -> None:
        # AccelStepper DRIVER mode: negative emits HIGH only when DIR inversion
        # is true; positive emits LOW when inversion is true.
        upstream_home_sign = (-1, -1, -1, 1, -1, -1)
        inversion = (False, False, True, True, True, True)
        actual_high = tuple(
            (sign < 0 and inverted) or (sign > 0 and not inverted)
            for sign, inverted in zip(upstream_home_sign, inversion)
        )
        self.assertEqual(
            (False, False, True, False, True, True),
            actual_high,
        )
        adapter = (ROOT / "src/octopus_board.cpp").read_text()
        self.assertIn(
            "false, false, true, true, true, true",
            adapter,
        )

    def test_original_current_hold_and_microstep_profile(self) -> None:
        constants = (ROOT / "src/constants.h").read_text()
        motor = (ROOT / "src/motor_init.cpp").read_text()
        main = (ROOT / "src/main.cpp").read_text()
        for requested in (2000, 2000, 1900, 1700, 1700, 965):
            self.assertIn(str(requested), constants)
        for hold in ("0.7", "0.8", "0.88"):
            self.assertIn(f"hold_multiplier = {hold}", motor)
        self.assertIn("motor_max_current * 0.85", main)
        self.assertIn("#define MICROSTEP 32", constants)
        self.assertIn("driver[num].en_pwm_mode(1)", main)
        self.assertIn("driver[num].toff(4)", main)
        self.assertIn("driver[num].blank_time(24)", main)
        self.assertIn("driver[num].pwm_autoscale(1)", main)

    def test_bootloader_clock_and_usb_contract(self) -> None:
        board = json.loads((ROOT / "boards/genericSTM32F446ZE.json").read_text())
        self.assertEqual("0x8000", board["build"]["flash_offset"])
        self.assertEqual("0x08008000", board["upload"]["offset_address"])
        self.assertEqual("stm32f446zet6", board["build"]["mcu"])
        platformio = (ROOT / "platformio.ini").read_text()
        self.assertIn("ststm32@19.6.0", platformio)
        self.assertIn("HSE_VALUE=12000000U", platformio)
        self.assertIn("PIO_FRAMEWORK_ARDUINO_ENABLE_CDC", platformio)
        # Preserve the already commissioned USB identity; Exp23 changes only
        # the pneumatic power-output pin mapping.
        self.assertIn("PAROL6_OCTOPUS_EXP22", platformio)
        self.assertIn("monitor_speed = 3000000", platformio)
        clock = (ROOT / "src/octopus_clock.cpp").read_text()
        for token in (
            "PLLM = 6",
            "PLLN = 180",
            "PLLP = RCC_PLLP_DIV2",
            "PLLSAIN = 96",
        ):
            self.assertIn(token, clock)

    def test_stop7_is_the_only_runtime_policy_extension(self) -> None:
        adapter = (ROOT / "src/octopus_board.cpp").read_text()
        self.assertIn("command == 103", adapter)
        self.assertIn("command == 101 && g_stop7_reset_seen", adapter)
        self.assertIn("robot->disabled = 1", adapter)
        self.assertIn("pinMode(kStepPins[slot], INPUT_PULLDOWN)", adapter)
        restore = adapter[
            adapter.index("void Restore_Step_Outputs") :
            adapter.index("} // namespace")
        ]
        self.assertLess(
            restore.index("Preload_Output(kStepPins[slot], LOW)"),
            restore.index("g_step_outputs_gated = false"),
        )
        # STOP7 hold must not deassert torque.
        hold = adapter[
            adapter.index("void Enforce_STOP7_Hold") :
            adapter.index("void Poll_STOP7")
        ]
        self.assertNotIn("Set_Mapped_Drivers_Enabled(false)", hold)
        for forbidden in (
            "verifiedJointMask",
            "hostFrameTimeout",
            "BootResetRequired",
            "runtime driver",
        ):
            self.assertNotIn(forbidden, adapter)


if __name__ == "__main__":
    unittest.main()
