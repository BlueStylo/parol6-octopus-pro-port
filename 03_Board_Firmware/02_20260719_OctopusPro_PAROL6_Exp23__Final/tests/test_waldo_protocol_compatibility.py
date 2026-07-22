from __future__ import annotations

import os
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PYTHON_API_ENV = os.environ.get("PAROL6_PYTHON_API_DIR")
PYTHON_API = (
    Path(PYTHON_API_ENV).expanduser().resolve()
    if PYTHON_API_ENV
    else None
)
API_AVAILABLE = bool(
    PYTHON_API is not None and (PYTHON_API / "parol6").is_dir()
)

if API_AVAILABLE:
    sys.path.insert(0, str(PYTHON_API))
    import numpy as np  # noqa: E402
    from parol6.protocol import wire  # noqa: E402
else:
    np = None
    wire = None


@unittest.skipUnless(
    API_AVAILABLE,
    "set PAROL6_PYTHON_API_DIR to a PAROL6 Python API v0.3.1 checkout",
)
class WaldoProtocolCompatibilityTest(unittest.TestCase):
    def test_host_frame_matches_original_56_byte_layout(self) -> None:
        positions = np.asarray([-1, 0, 1, 1000, -1000, 123456], dtype=np.int32)
        speeds = np.asarray([1, -2, 3, -4, 5, -6], dtype=np.int32)
        affected = np.asarray([1, 0, 0, 0, 0, 0, 0, 0], dtype=np.uint8)
        io = np.asarray([0, 1, 0, 1, 0, 1, 0, 1], dtype=np.uint8)
        gripper = np.asarray([123, 45, 67, 3, 0, 5], dtype=np.int32)
        frame = bytearray(56)
        wire.pack_tx_frame_into(
            memoryview(frame),
            positions,
            speeds,
            wire.CommandCode.CLEAR_ERROR,
            affected,
            io,
            7,
            gripper,
        )
        self.assertEqual(b"\xff\xff\xff\x34", bytes(frame[:4]))
        self.assertEqual(103, frame[40])
        self.assertEqual(0x80, frame[41])
        self.assertEqual(0x55, frame[42])
        self.assertEqual(0xE4, frame[53])
        self.assertEqual(b"\x01\x02", bytes(frame[-2:]))

    def test_board_payload_matches_original_60_byte_frame_contract(self) -> None:
        payload = bytearray(52)
        positions = (-1, 0, 1, 1000, -1000, 123456)
        speeds = (0, 5, -5, 9999, -9999, 654321)
        offset = 0
        for value in positions + speeds:
            payload[offset : offset + 3] = bytes(wire.split_to_3_bytes(value))
            offset += 3
        payload[36] = 0xFF
        payload[37] = 0xAA
        payload[38] = 0
        payload[39] = 0
        payload[40] = 0x12
        payload[41] = 0x34
        payload[44] = 7
        payload[45:51] = b"\x01\x02\x00\x64\x00\x0a"
        payload[51] = 0b00001100

        pos = np.zeros(6, dtype=np.int32)
        spd = np.zeros(6, dtype=np.int32)
        homed = np.zeros(8, dtype=np.uint8)
        io = np.zeros(8, dtype=np.uint8)
        temp = np.zeros(8, dtype=np.uint8)
        poserr = np.zeros(8, dtype=np.uint8)
        timing = np.zeros(1, dtype=np.int32)
        grip = np.zeros(6, dtype=np.int32)
        self.assertTrue(
            wire.unpack_rx_frame_into(
                memoryview(payload),
                pos,
                spd,
                homed,
                io,
                temp,
                poserr,
                timing,
                grip,
            )
        )
        self.assertEqual(list(positions), pos.tolist())
        self.assertEqual(list(speeds), spd.tolist())
        self.assertEqual([1] * 8, homed.tolist())
        self.assertEqual([1, 0, 1, 0, 1, 0, 1, 0], io.tolist())
        self.assertEqual(0x1234, int(timing[0]))
        self.assertEqual([7, 258, 100, 10, 12, 3], grip.tolist())

    def test_firmware_declares_original_packet_lengths(self) -> None:
        main = (ROOT / "src/main.cpp").read_text()
        self.assertIn("int len = 56;", main)
        self.assertIn("data_len = input_byte;", main)
        self.assertIn("data_buffer[data_len - 2] == end_cond1_byte", main)
        self.assertIn("Command = data_buffer[36];", main)

    def test_waldo_reset_edge_and_stop_ack_contract(self) -> None:
        utility = (
            PYTHON_API
            / "parol6"
            / "commands"
            / "utility_commands.py"
        ).read_text()
        self.assertIn(
            "CommandCode.IDLE,\n"
            "            CommandCode.CLEAR_ERROR,\n"
            "            CommandCode.IDLE,",
            utility,
        )
        firmware = (ROOT / "src/main.cpp").read_text()
        adapter = (ROOT / "src/octopus_board.cpp").read_text()
        self.assertIn("STOP7_Reset_Acknowledged()", firmware)
        self.assertIn(
            "command == 255 && g_stop7_reset_seen",
            adapter,
        )
        self.assertIn("return g_stop7_reset_seen;", adapter)

    def test_pneumatic_tool_port1_reaches_fan0_output1(self) -> None:
        gripper_commands = (
            PYTHON_API
            / "parol6"
            / "commands"
            / "gripper_commands.py"
        ).read_text()
        pins = (ROOT / "src/iodefs.h").read_text()
        firmware = (ROOT / "src/main.cpp").read_text()

        self.assertIn(
            'self._state_to_set = 1 if self.p.action == "open" else 0',
            gripper_commands,
        )
        self.assertIn(
            "self._port_index = 2 if self.p.port == 1 else 3",
            gripper_commands,
        )
        self.assertIn("#define FAN0 PA8", pins)
        self.assertIn("#define OUTPUT1 FAN0", pins)
        self.assertIn(
            "digitalWrite(OUTPUT1, PAROL6.commanded_OUT1);",
            firmware,
        )


if __name__ == "__main__":
    unittest.main()
