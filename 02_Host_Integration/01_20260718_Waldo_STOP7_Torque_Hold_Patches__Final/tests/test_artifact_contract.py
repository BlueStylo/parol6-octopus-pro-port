from __future__ import annotations

import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
API_PATCH = ROOT / "patches" / "0001-parol6-api-v0.3.1-clear-error-103.patch"
WALDO_PATCH = (
    ROOT / "patches" / "0002-waldo-commander-exp19-reset-handshake.patch"
)
LOCK = ROOT / "upstream.lock"


class CompatibilityArtifactTest(unittest.TestCase):
    @staticmethod
    def _added_lines(patch: str) -> str:
        return "\n".join(
            line[1:]
            for line in patch.splitlines()
            if line.startswith("+") and not line.startswith("+++")
        )

    def test_upstream_versions_are_pinned(self) -> None:
        lock = LOCK.read_text(encoding="utf-8")
        self.assertIn("PAROL6_PYTHON_API_VERSION=v0.3.1", lock)
        self.assertIn(
            "PAROL6_PYTHON_API_COMMIT=8f6b3d19ddd3fb854af9890d89c7c96ca093cb7c",
            lock,
        )
        self.assertIn(
            "WALDO_COMMANDER_COMMIT=72d7ced4da1a8b0ababd4ca9a39a0693523b098f",
            lock,
        )

    def test_api_patch_has_one_shot_wire_queue_and_safe_reset(self) -> None:
        patch = API_PATCH.read_text(encoding="utf-8")
        added = self._added_lines(patch)

        # CLEAR_ERROR is serialized through the existing frame writer and
        # remains disarmed until a separate ENABLE command is accepted.
        self.assertIn("CLEAR_ERROR = 103", added)
        self.assertIn("_firmware_command_queue", added)
        self.assertIn("CommandCode.CLEAR_ERROR,", added)
        self.assertIn("first_frame_received", added)
        self.assertIn("if queue_ready:", added)
        self.assertIn("self._firmware_command_queue.popleft()", added)
        self.assertIn("state.enabled = False", added)
        self.assertIn("Firmware reset; RESUME required", added)
        self.assertIn(
            "A new STOP7/boot safety-latch transition preempts",
            added,
        )
        self.assertIn("Never replay a one-shot command", added)
        self.assertIn("A manually selected device starts", added)

        # Digital HALT is a torque-holding stop. Physical STOP7 also must not
        # be converted into DISABLE, which would drop a gravity-loaded axis.
        self.assertIn(
            "self._firmware_commands = (CommandCode.JOG, CommandCode.IDLE)",
            added,
        )
        self.assertIn("Do not send DISABLE here", added)
        self.assertNotIn(
            "state.Command_out = CommandCode.DISABLE",
            added,
        )
        self.assertIn(
            "STOP7 / E-STOP latched; RESET and RESUME required",
            added,
        )
        self.assertIn(
            "test_estop_preempts_queued_enable_without_disabling_firmware",
            added,
        )

        # Cancel is a generation boundary, so a segment returned late by the
        # planner cannot be replayed after reset/resume.
        self.assertIn("self._generation += 1", added)
        self.assertIn("Discarded stale planner segment", added)
        self.assertIn(
            "test_cancel_drops_late_old_generation_and_accepts_current",
            added,
        )
        self.assertIn("tests/integration/conftest.py", patch)
        self.assertIn("test_reset_command.py", patch)

    def test_waldo_patch_waits_and_verifies_before_resume(self) -> None:
        patch = WALDO_PATCH.read_text(encoding="utf-8")
        added = self._added_lines(patch)
        self.assertIn("_reset_and_verify_hardware_safe", added)
        self.assertIn("await client.reset()", added)
        self.assertIn("await asyncio.sleep(0.06)", patch)
        self.assertIn("await asyncio.sleep(0.10)", patch)
        self.assertIn("io_state[4] == 1", patch)
        self.assertIn("BootResetRequired/EstopOpen", added)
        self.assertNotIn("every safety latch cleared", added)
        self.assertNotIn("composite", added.lower())
        self.assertIn(
            "Confirm the physical E-STOP is released, then use",
            added,
        )
        self.assertIn("self._last_io_state: int | None = None", added)
        self.assertIn("if self._last_io_state is None:", added)
        self.assertIn("safety reset required at startup", added)
        self.assertIn(
            "test_first_zero_ack_is_not_logged_as_a_physical_stop_edge",
            added,
        )
        self.assertIn("Reset was not accepted", patch)
        self.assertIn("Awaiting explicit Reset & Resume", added)
        self.assertIn("await client.halt()", added)
        self.assertIn(
            "test_digital_stop_resumes_without_resetting_firmware",
            added,
        )
        self.assertIn("test_real_hardware_startup_never_auto_resumes", patch)
        self.assertIn(
            "test_manual_simulator_to_hardware_never_auto_resumes",
            added,
        )
        self.assertIn("WALDO_DISABLE_CAMERA", added)


if __name__ == "__main__":
    unittest.main()
