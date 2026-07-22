#!/usr/bin/env python3
"""Compile and execute Exp23's literal production HOME state machine.

The test deliberately does not keep a hand-copied version of ``home_all()``.
It extracts ``reset_homing()`` and ``home_all()`` from production ``src/main.cpp``
on every run, first proves that both are byte-for-byte equal to the pinned
upstream snapshot, and then compiles those exact function strings into a small
native fake-motion runtime.
"""

from __future__ import annotations

import hashlib
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
PRODUCTION_MAIN = ROOT / "src" / "main.cpp"
UPSTREAM_ENV = os.environ.get("PAROL6_UPSTREAM_FIRMWARE_DIR")
UPSTREAM_MAIN = (
    Path(UPSTREAM_ENV).expanduser().resolve() / "src" / "main.cpp"
    if UPSTREAM_ENV
    else None
)


def extract_cpp_function(text: str, signature: str) -> str:
    match = re.search(signature + r"\s*\{", text, re.MULTILINE)
    if match is None:
        raise RuntimeError(f"function not found: {signature}")

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

    raise RuntimeError(f"unterminated function: {signature}")


HARNESS_PREFIX = r"""
#include <array>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {

constexpr int kJointCount = 6;

struct MotorStruct {
  long standby_position = 0;
  long homed_position = 0;
  int homed = 0;
  int homing_stage_1 = 0;
  int homing_stage_2 = 0;
  int limit_switch_trigger = 0;
  int LIMIT = 0;
};

struct StepperMetrics {
  long set_speed_calls = 0;
  long move_to_calls = 0;
  long run_speed_calls = 0;
  long run_speed_to_position_calls = 0;
  long run_calls = 0;
  long set_current_position_calls = 0;
  long set_acceleration_calls = 0;
};

class FakeStepper {
 public:
  void setSpeed(double value) {
    speed_ = value;
    ++metrics_.set_speed_calls;
  }

  void moveTo(long value) {
    target_position_ = value;
    ++metrics_.move_to_calls;
  }

  void runSpeed() {
    ++metrics_.run_speed_calls;
  }

  void runSpeedToPosition() {
    // The native harness advances a complete bounded segment per scheduler
    // call. The production state transitions and target values stay literal.
    current_position_ = target_position_;
    ++metrics_.run_speed_to_position_calls;
  }

  void run() {
    current_position_ = target_position_;
    ++metrics_.run_calls;
  }

  void setCurrentPosition(long value) {
    current_position_ = value;
    target_position_ = value;
    ++metrics_.set_current_position_calls;
  }

  long currentPosition() const {
    return current_position_;
  }

  void setAcceleration(double value) {
    acceleration_ = value;
    ++metrics_.set_acceleration_calls;
  }

  const StepperMetrics& metrics() const {
    return metrics_;
  }

 private:
  long current_position_ = 0;
  long target_position_ = 0;
  double speed_ = 0;
  double acceleration_ = 0;
  StepperMetrics metrics_{};
};

MotorStruct Joint[kJointCount];
FakeStepper stepper[kJointCount];

int homed = 0;
int j5_homing_offset = 8900;

int run_once = 0;
int joint123_stage1 = 0;
int joint123_stage2 = 0;
int joint123_stage3 = 0;
int joint123_done = 0;

int J4_stage2 = 0;
int J4_stage3 = 0;
int J4_stage4 = 0;
int J4_done = 0;

int J6_stage2 = 0;
int J6_stage3 = 0;
int J6_stage4 = 0;
int J6_done = 0;

int J5_stage2 = 0;
int J5_stage3 = 0;
int J5_stage4 = 0;
int J5_done = 0;

int joint456_stage1 = 0;
int joint456_stage2 = 0;
int joint456_stage3 = 0;

std::array<int, kJointCount> limit_reads{};
std::array<int, kJointCount> opposite_reads{};
std::array<int, kJointCount> trigger_reads{};

void require(bool condition, const std::string& message) {
  if (!condition) {
    std::cerr << "FAIL: " << message << '\n';
    std::exit(1);
  }
}

int digitalRead(int pin) {
  require(pin >= 0 && pin < kJointCount, "unexpected LIMIT pin");
  const int joint = pin;
  ++limit_reads[joint];

  // On each HOME cycle, every joint first reports the non-trigger level. This
  // proves that the production equality check does not advance a stage on the
  // wrong polarity. All following reads report the configured trigger level,
  // allowing both the coarse and reference hits to complete.
  if (limit_reads[joint] == 1) {
    ++opposite_reads[joint];
    return 1 - Joint[joint].limit_switch_trigger;
  }
  ++trigger_reads[joint];
  return Joint[joint].limit_switch_trigger;
}

void initializeJoints() {
  constexpr std::array<long, kJointCount> standby = {
      10240, -32000, 57905, 0, 0, 32000,
  };
  constexpr std::array<long, kJointCount> home_offset = {
      13500, 19588, 23020, -10200, 8900, 15900,
  };
  constexpr std::array<int, kJointCount> trigger = {
      0, 1, 1, 0, 1, 0,
  };

  for (int joint = 0; joint < kJointCount; ++joint) {
    Joint[joint].standby_position = standby[joint];
    Joint[joint].homed_position = home_offset[joint];
    Joint[joint].limit_switch_trigger = trigger[joint];
    Joint[joint].LIMIT = joint;
  }
}

void resetLimitScript() {
  limit_reads.fill(0);
  opposite_reads.fill(0);
  trigger_reads.fill(0);
}

StepperMetrics subtract(const StepperMetrics& after,
                        const StepperMetrics& before) {
  return {
      after.set_speed_calls - before.set_speed_calls,
      after.move_to_calls - before.move_to_calls,
      after.run_speed_calls - before.run_speed_calls,
      after.run_speed_to_position_calls -
          before.run_speed_to_position_calls,
      after.run_calls - before.run_calls,
      after.set_current_position_calls -
          before.set_current_position_calls,
      after.set_acceleration_calls - before.set_acceleration_calls,
  };
}

}  // namespace
"""


HARNESS_SUFFIX = r"""

namespace {

void verifyResetHomingBody() {
  run_once = 9;
  joint123_stage1 = 9;
  joint123_stage2 = 9;
  joint123_stage3 = 9;
  joint123_done = 9;
  J4_stage2 = 9;
  J4_stage3 = 9;
  J4_stage4 = 9;
  J4_done = 9;
  J6_stage2 = 9;
  J6_stage3 = 9;
  J6_stage4 = 9;
  J6_done = 9;
  J5_stage2 = 9;
  J5_stage3 = 9;
  J5_stage4 = 9;
  J5_done = 9;
  joint456_stage1 = 9;
  joint456_stage2 = 9;
  joint456_stage3 = 9;

  reset_homing();

  require(run_once == 0, "reset_homing run_once");
  require(joint123_stage1 == 0 && joint123_stage2 == 0 &&
              joint123_stage3 == 0 && joint123_done == 0,
          "reset_homing J1-J3 stages");
  require(J4_stage2 == 0 && J4_stage3 == 0 &&
              J4_stage4 == 0 && J4_done == 0,
          "reset_homing J4 stages");
  require(J6_stage2 == 0 && J6_stage3 == 0 &&
              J6_stage4 == 0 && J6_done == 0,
          "reset_homing J6 stages");
  require(J5_stage2 == 0 && J5_stage3 == 0 &&
              J5_stage4 == 0 && J5_done == 0,
          "reset_homing J5 stages");
  require(joint456_stage1 == 0 && joint456_stage2 == 0 &&
              joint456_stage3 == 0,
          "reset_homing legacy J4-J6 stages");
}

void runHomeCycle(const std::string& label) {
  std::array<StepperMetrics, kJointCount> before{};
  for (int joint = 0; joint < kJointCount; ++joint) {
    before[joint] = stepper[joint].metrics();
    require(Joint[joint].homing_stage_1 == 0,
            label + ": stage 1 did not start reset");
    require(Joint[joint].homing_stage_2 == 0,
            label + ": stage 2 did not start reset");
  }

  resetLimitScript();
  homed = 0;  // Mirrors reception of upstream command 100.

  require(home_all() == 0, label + ": HOME completed on first scheduler call");
  for (int joint = 0; joint < 3; ++joint) {
    require(limit_reads[joint] == 1,
            label + ": first limit sample missing for J" +
                std::to_string(joint + 1));
    require(opposite_reads[joint] == 1,
            label + ": first sample was not opposite polarity");
    require(Joint[joint].homing_stage_1 == 0,
            label + ": wrong polarity advanced J" +
                std::to_string(joint + 1));
  }

  int scheduler_calls = 1;
  while (homed == 0 && scheduler_calls < 100) {
    home_all();
    ++scheduler_calls;
  }

  require(homed == 1, label + ": HOME did not complete");
  require(scheduler_calls < 100, label + ": scheduler guard exhausted");

  for (int joint = 0; joint < kJointCount; ++joint) {
    const StepperMetrics delta =
        subtract(stepper[joint].metrics(), before[joint]);
    const std::string joint_label =
        label + " J" + std::to_string(joint + 1);

    require(limit_reads[joint] >= 3,
            joint_label + ": did not execute mismatch/coarse/reference reads");
    require(opposite_reads[joint] == 1,
            joint_label + ": wrong-polarity probe count");
    require(trigger_reads[joint] >= 2,
            joint_label + ": both trigger transitions were not exercised");
    require(delta.run_speed_calls >= 3,
            joint_label + ": limit-search stepping was not repeated");
    require(delta.run_speed_to_position_calls >= 1,
            joint_label + ": switch back-off stage was not repeated");
    require(delta.move_to_calls >= 2,
            joint_label + ": back-off and standby targets were not repeated");
    require(delta.run_calls >= 1,
            joint_label + ": standby motion was not repeated");
    require(delta.set_current_position_calls >= 3,
            joint_label + ": reference/final position updates missing");
    require(stepper[joint].currentPosition() ==
                Joint[joint].standby_position,
            joint_label + ": final standby coordinate mismatch");
    require(Joint[joint].homed == 1,
            joint_label + ": joint homed flag missing");
    require(Joint[joint].homing_stage_1 == 0 &&
                Joint[joint].homing_stage_2 == 0,
            joint_label + ": joint stages not cleared after HOME");
  }

  require(run_once == 0, label + ": run_once not reset at completion");
  require(joint123_done == 0 && J4_done == 0 &&
              J6_done == 0 && J5_done == 0,
          label + ": completion stages not reset");
}

}  // namespace

int main() {
  initializeJoints();
  verifyResetHomingBody();

  runHomeCycle("first HOME");
  runHomeCycle("repeat HOME");

  std::cout
      << "Exp23 literal upstream home_all/reset_homing native regression: PASS\n";
  return 0;
}
"""


def main() -> int:
    if UPSTREAM_MAIN is None or not UPSTREAM_MAIN.is_file():
        print(
            "SKIP: set PAROL6_UPSTREAM_FIRMWARE_DIR to the pinned "
            "upstream firmware directory"
        )
        return 0

    production_text = PRODUCTION_MAIN.read_text(encoding="utf-8")
    upstream_text = UPSTREAM_MAIN.read_text(encoding="utf-8")
    signatures = {
        "reset_homing": r"void\s+reset_homing\s*\(\s*\)",
        "home_all": r"int\s+home_all\s*\(\s*\)",
    }

    production_functions: dict[str, str] = {}
    for name, signature in signatures.items():
        production_function = extract_cpp_function(production_text, signature)
        upstream_function = extract_cpp_function(upstream_text, signature)
        if production_function != upstream_function:
            raise RuntimeError(
                f"production {name} is not byte-for-byte pinned upstream"
            )
        production_functions[name] = production_function

    generated = (
        HARNESS_PREFIX
        + '\n#line 961 "src/main.cpp"\n'
        + production_functions["reset_homing"]
        + '\n#line 990 "src/main.cpp"\n'
        + production_functions["home_all"]
        + "\n"
        + HARNESS_SUFFIX
    )

    build_dir = (
        Path(os.environ.get("TMPDIR", tempfile.gettempdir()))
        / "parol6-exp23-upstream-home-native"
    )
    build_dir.mkdir(parents=True, exist_ok=True)
    generated_cpp = build_dir / "literal_upstream_home_test.cpp"
    executable = build_dir / "literal_upstream_home_test"
    generated_cpp.write_text(generated, encoding="utf-8")

    compiler = os.environ.get("CXX", "c++")
    subprocess.run(
        [
            compiler,
            "-std=c++17",
            "-Wall",
            "-Wextra",
            "-Wno-unused-value",
            "-pedantic",
            str(generated_cpp),
            "-o",
            str(executable),
        ],
        check=True,
    )
    subprocess.run([str(executable)], check=True)

    for name in ("reset_homing", "home_all"):
        digest = hashlib.sha256(
            production_functions[name].encode("utf-8")
        ).hexdigest()
        print(f"{name} production/upstream SHA-256: {digest}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
