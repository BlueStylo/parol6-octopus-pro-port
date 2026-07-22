from __future__ import annotations

import hashlib
import os
import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
UPSTREAM_ENV = os.environ.get("PAROL6_UPSTREAM_FIRMWARE_DIR")
UPSTREAM = (
    Path(UPSTREAM_ENV).expanduser().resolve()
    if UPSTREAM_ENV
    else None
)
UPSTREAM_AVAILABLE = bool(
    UPSTREAM is not None and (UPSTREAM / "src" / "main.cpp").is_file()
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def extract_cpp_function(text: str, signature: str) -> str:
    match = re.search(signature + r"\s*\{", text, re.MULTILINE)
    if match is None:
        raise AssertionError(f"function not found: {signature}")
    brace = text.find("{", match.start())
    depth = 0
    index = brace
    state = "code"
    quote = ""
    while index < len(text):
        char = text[index]
        nxt = text[index + 1] if index + 1 < len(text) else ""
        if state == "line_comment":
            if char == "\n":
                state = "code"
        elif state == "block_comment":
            if char == "*" and nxt == "/":
                state = "code"
                index += 1
        elif state == "string":
            if char == "\\":
                index += 1
            elif char == quote:
                state = "code"
        else:
            if char == "/" and nxt == "/":
                state = "line_comment"
                index += 1
            elif char == "/" and nxt == "*":
                state = "block_comment"
                index += 1
            elif char in {'"', "'"}:
                state = "string"
                quote = char
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return text[match.start() : index + 1]
        index += 1
    raise AssertionError(f"unterminated function: {signature}")


@unittest.skipUnless(
    UPSTREAM_AVAILABLE,
    "set PAROL6_UPSTREAM_FIRMWARE_DIR to the upstream firmware directory",
)
class LiteralUpstreamDeltaTest(unittest.TestCase):
    def test_upstream_snapshot_is_present(self) -> None:
        assert UPSTREAM is not None
        self.assertTrue((UPSTREAM / "src/main.cpp").is_file())
        lock = (ROOT / "UPSTREAM.lock").read_text(encoding="utf-8")
        self.assertIn(
            'pinned_commit = "896fa9fd793a076a0b3790b8a4a29a6f5bb12fcb"',
            lock,
        )

    def test_every_unadapted_upstream_file_is_byte_identical(self) -> None:
        assert UPSTREAM is not None
        allowed_deltas = {
            Path("src/iodefs.h"),
            Path("src/main.cpp"),
        }
        for source in sorted((UPSTREAM / "src").rglob("*")):
            if not source.is_file():
                continue
            relative = source.relative_to(UPSTREAM)
            if relative in allowed_deltas:
                continue
            candidate = ROOT / relative
            self.assertTrue(candidate.is_file(), relative.as_posix())
            self.assertEqual(
                sha256(source),
                sha256(candidate),
                f"unexpected upstream delta: {relative}",
            )

    def test_motion_protocol_and_home_functions_are_exact_upstream(self) -> None:
        assert UPSTREAM is not None
        upstream_main = (UPSTREAM / "src/main.cpp").read_text()
        candidate_main = (ROOT / "src/main.cpp").read_text()
        signatures = (
            r"void\s+Unpack_data\s*\(\s*uint8_t\s*\*\s*data_buffer\s*\)",
            r"void\s+Pack_data\s*\(\s*\)",
            r"void\s+reset_homing\s*\(\s*\)",
            r"int\s+home_all\s*\(\s*\)",
            r"void\s+Init_motor_direction\s*\(\s*\)",
            r"void\s+Init_motor_drivers\s*\(\s*int\s+num\s*\)",
        )
        for signature in signatures:
            self.assertEqual(
                extract_cpp_function(upstream_main, signature),
                extract_cpp_function(candidate_main, signature),
                signature,
            )

    def test_original_command_blocks_remain_literal(self) -> None:
        assert UPSTREAM is not None
        upstream_main = (UPSTREAM / "src/main.cpp").read_text()
        candidate_main = (ROOT / "src/main.cpp").read_text()
        signatures = (
            r"if\s*\(\s*PAROL6\.command\s*==\s*255\s*\)",
            r"if\s*\(\s*PAROL6\.command\s*==\s*101\s*\)",
            r"if\s*\(\s*PAROL6\.command\s*==\s*102\s*\)",
            r"if\s*\(\s*PAROL6\.command\s*==\s*103\s*\)",
            r"if\s*\(\s*PAROL6\.command\s*==\s*100\s*\)",
            r"if\s*\(\s*PAROL6\.command\s*==\s*123\s*\)",
            r"if\s*\(\s*PAROL6\.command\s*==\s*156\s*\)",
        )
        for signature in signatures:
            original = extract_cpp_function(upstream_main, signature)
            self.assertIn(original, candidate_main, signature)

    def test_stop7_recovery_commits_only_at_upstream_dispatch(self) -> None:
        main = (ROOT / "src/main.cpp").read_text()
        enable_block = extract_cpp_function(
            main,
            r"if\s*\(\s*PAROL6\.command\s*==\s*101\s*\)",
        )
        enable_end = main.index(enable_block) + len(enable_block)
        commit = main.index(
            "octopus_board::Commit_STOP7_Recovery(PAROL6.command)"
        )
        enforce = main.index("octopus_board::Enforce_STOP7_Hold")
        motion_guard = main.index("if (PAROL6.disabled == 0)")
        self.assertLess(enable_end, commit)
        self.assertLess(commit, enforce)
        self.assertLess(enforce, motion_guard)

        unpack = main.index("Unpack_data(data_buffer);")
        record = main.index(
            "octopus_board::Record_Valid_Command_Frame(PAROL6.command)"
        )
        self.assertLess(unpack, record)

    def test_original_joint_profiles_and_codecs_are_byte_identical(self) -> None:
        assert UPSTREAM is not None
        for relative in (
            "src/motor_init.cpp",
            "src/constants.h",
            "src/structs.h",
            "src/utils.cpp",
        ):
            self.assertEqual(
                sha256(UPSTREAM / relative),
                sha256(ROOT / relative),
                relative,
            )

    def test_adapter_hooks_are_named_and_bounded(self) -> None:
        main = (ROOT / "src/main.cpp").read_text()
        begin = re.findall(r"OCTOPUS_ADAPTER_BEGIN\(([^)]+)\)", main)
        end = re.findall(r"OCTOPUS_ADAPTER_END\(([^)]+)\)", main)
        expected = {
            "direction",
            "enable_off",
            "spi",
            "unavailable_peripherals",
            "cs_idle",
            "enable_on",
            "stop7_poll",
            "stop7_enforce",
            "stop7_command_edge",
            "stop7_status",
        }
        self.assertEqual(expected, set(begin))
        self.assertEqual(begin, end)
        self.assertLessEqual(len(main.splitlines()) - 1421, 70)

    def test_no_exp21_reimplementation_is_linked(self) -> None:
        platformio = (ROOT / "platformio.ini").read_text()
        for forbidden in (
            "port_motion.cpp",
            "port_safety.cpp",
            "parol6_protocol.cpp",
            "P6_COMMISSIONING_GATE",
            "P6_ORIGINAL_RUNTIME_SEMANTICS",
        ):
            self.assertNotIn(forbidden, platformio)


if __name__ == "__main__":
    unittest.main()
