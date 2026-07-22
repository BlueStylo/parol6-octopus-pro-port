from __future__ import annotations

import sys
import unittest
from pathlib import Path

HOST_WALDO = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(HOST_WALDO))

from clear_error_contract import (  # noqa: E402
    AFFECTED_JOINT_OFFSET,
    COMMAND_CLEAR_ERROR,
    COMMAND_ENABLE,
    COMMAND_IDLE,
    COMMAND_OFFSET,
    COMPATIBILITY_OFFSET,
    HOST_FRAME_SIZE,
    TIMEOUT_OFFSET,
    boot_reset_handshake,
    build_zero_motion_frame,
    frames_for,
)


class ClearErrorWireContractTest(unittest.TestCase):
    def test_clear_error_is_exact_legacy_56_byte_frame(self) -> None:
        frame = build_zero_motion_frame(COMMAND_CLEAR_ERROR)

        self.assertEqual(len(frame), HOST_FRAME_SIZE)
        self.assertEqual(frame[0:4], b"\xff\xff\xff\x34")
        self.assertEqual(frame[COMMAND_OFFSET], 103)
        self.assertEqual(frame[AFFECTED_JOINT_OFFSET], 0)
        self.assertEqual(frame[TIMEOUT_OFFSET], 0)
        self.assertEqual(frame[COMPATIBILITY_OFFSET], 0xE4)
        self.assertEqual(frame[-2:], b"\x01\x02")

        zero_regions = frame[4:COMMAND_OFFSET] + frame[41:53]
        self.assertEqual(zero_regions, bytes(len(zero_regions)))

    def test_arm_reset_release_enable_are_four_ordered_frames(self) -> None:
        commands = boot_reset_handshake()
        self.assertEqual(
            commands,
            (
                COMMAND_IDLE,
                COMMAND_CLEAR_ERROR,
                COMMAND_IDLE,
                COMMAND_ENABLE,
            ),
        )
        frames = frames_for(commands)
        self.assertEqual(
            [frame[COMMAND_OFFSET] for frame in frames],
            [255, 103, 255, 101],
        )
        self.assertEqual(len(set(frames)), 3)

    def test_out_of_range_command_is_rejected(self) -> None:
        for command in (-1, 256):
            with self.subTest(command=command):
                with self.assertRaises(ValueError):
                    build_zero_motion_frame(command)


if __name__ == "__main__":
    unittest.main()
