"""Dependency-free reference for the PAROL6 binary CLEAR_ERROR handshake.

This is not a second serial implementation.  It exists only to make the exact
wire bytes and reset ordering reviewable without installing Waldo Commander or
PAROL6-python-API.
"""

from __future__ import annotations

from collections.abc import Iterable

HOST_FRAME_SIZE = 56
HOST_PAYLOAD_SIZE = 52
COMMAND_OFFSET = 40
AFFECTED_JOINT_OFFSET = 41
TIMEOUT_OFFSET = 43
COMPATIBILITY_OFFSET = 53

COMMAND_CLEAR_ERROR = 103
COMMAND_ENABLE = 101
COMMAND_IDLE = 255


def build_zero_motion_frame(command: int) -> bytes:
    """Build the legacy 56-byte host frame with zero motion/tool payloads."""
    if not 0 <= command <= 0xFF:
        raise ValueError("command must fit in one byte")

    frame = bytearray(HOST_FRAME_SIZE)
    frame[0:3] = b"\xff\xff\xff"
    frame[3] = HOST_PAYLOAD_SIZE
    frame[COMMAND_OFFSET] = command
    frame[COMPATIBILITY_OFFSET] = 0xE4
    frame[54:56] = b"\x01\x02"
    return bytes(frame)


def boot_reset_handshake() -> tuple[int, int, int, int]:
    """Return the deliberate boot recovery order.

    The leading IDLE standardizes reset-edge arming for both boot and later
    STOP7 events. At boot, CLEAR_ERROR leaves the firmware READY_DISARMED. After
    a protective STOP7 torque hold, it leaves STOP_HOLD_RESUME_REQUIRED with
    the verified held mask still enabled. The following IDLE releases/re-arms
    the reset edge, and ENABLE is a separate action that is permitted only
    after fresh reset-acknowledgement telemetry. Neither reset result permits
    motion until that explicit ENABLE.
    """
    return (
        COMMAND_IDLE,
        COMMAND_CLEAR_ERROR,
        COMMAND_IDLE,
        COMMAND_ENABLE,
    )


def frames_for(commands: Iterable[int]) -> tuple[bytes, ...]:
    """Build reference frames for a command sequence."""
    return tuple(build_zero_motion_frame(command) for command in commands)
