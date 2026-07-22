#include <Arduino.h>

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>

#include "hw_init.h"
#include "iodefs.h"
#include "octopus_board.h"

namespace fake_arduino {

std::array<std::uint8_t, kFakePinCount> levels = {};
std::array<std::uint8_t, kFakePinCount> modes = {};
std::array<InterruptCallback, kFakePinCount> callbacks = {};
std::array<int, kFakePinCount> interrupt_modes = {};
bool interrupts_enabled = true;
std::uint32_t interrupt_on_low_write_pin = kFakePinCount;

void reset() {
  levels.fill(HIGH);
  modes.fill(0U);
  callbacks.fill(nullptr);
  interrupt_modes.fill(0);
  interrupts_enabled = true;
  interrupt_on_low_write_pin = kFakePinCount;
}

void setInput(std::uint32_t pin, std::uint8_t level) {
  levels.at(pin) = level;
}

void fireInterrupt(std::uint32_t pin) {
  assert(callbacks.at(pin) != nullptr);
  callbacks.at(pin)();
}

}  // namespace fake_arduino

namespace {

constexpr std::array<std::uint32_t, 8> kStepBySlot = {{
    PF13, PG0, PF11, PG4, PF9, PC13, PE2, PE6,
}};
constexpr std::array<std::uint32_t, 8> kDirectionBySlot = {{
    PF12, PG1, PG3, PC1, PF10, PF0, PE3, PA14,
}};
constexpr std::array<std::uint32_t, 8> kEnableBySlot = {{
    PF14, PF15, PG5, PA0, PG2, PF1, PD4, PE0,
}};
constexpr std::array<std::uint32_t, 8> kChipSelectBySlot = {{
    PC4, PD11, PC6, PC7, PF2, PE4, PE1, PD3,
}};
constexpr std::array<std::uint8_t, 6> kJointToMotor = {{
    6, 5, 4, 3, 1, 0,
}};

void assertMappedEnableLevels(std::uint8_t mapped_level) {
  for (std::size_t slot = 0; slot < kEnableBySlot.size(); ++slot) {
    const bool bypassed = slot == 2U || slot == 7U;
    const std::uint8_t expected = bypassed ? HIGH : mapped_level;
    assert(fake_arduino::levels.at(kEnableBySlot.at(slot)) == expected);
  }
}

void assertEveryStepLow() {
  for (const std::uint32_t pin : kStepBySlot) {
    assert(fake_arduino::levels.at(pin) == LOW);
  }
}

void assertEveryStepMode(std::uint8_t mode) {
  for (const std::uint32_t pin : kStepBySlot) {
    assert(fake_arduino::modes.at(pin) == mode);
  }
}

void assertEveryChipSelectHigh() {
  for (const std::uint32_t pin : kChipSelectBySlot) {
    assert(fake_arduino::levels.at(pin) == HIGH);
  }
  assert(fake_arduino::levels.at(FLASH_SELECT) == HIGH);
}

void testLiteralJointPinMap() {
  // The untouched upstream constructor order is
  // PUL1/PUL6/PUL5/PUL4/PUL2/PUL3 for logical J1..J6.
  constexpr std::array<std::uint32_t, 6> logical_step = {{
      PUL1, PUL6, PUL5, PUL4, PUL2, PUL3,
  }};
  constexpr std::array<std::uint32_t, 6> logical_direction = {{
      DIR1, DIR6, DIR5, DIR4, DIR2, DIR3,
  }};
  constexpr std::array<std::uint32_t, 6> logical_enable = {{
      ENABLE1, ENABLE6, ENABLE5, ENABLE4, ENABLE2, ENABLE3,
  }};
  constexpr std::array<std::uint32_t, 6> logical_chip_select = {{
      SELECT1, SELECT6, SELECT5, SELECT4, SELECT2, SELECT3,
  }};
  constexpr std::array<std::uint32_t, 6> logical_limit = {{
      LIMIT6, LIMIT2, LIMIT3, LIMIT4, LIMIT5, LIMIT1,
  }};
  constexpr std::array<std::uint32_t, 6> expected_limit = {{
      PG11, PG10, PG9, PG12, PG6, PG13,
  }};

  for (std::size_t joint = 0; joint < kJointToMotor.size(); ++joint) {
    const std::size_t slot = kJointToMotor.at(joint);
    assert(logical_step.at(joint) == kStepBySlot.at(slot));
    assert(logical_direction.at(joint) == kDirectionBySlot.at(slot));
    assert(logical_enable.at(joint) == kEnableBySlot.at(slot));
    assert(logical_chip_select.at(joint) ==
           kChipSelectBySlot.at(slot));
    assert(logical_limit.at(joint) == expected_limit.at(joint));
  }
}

void testDirectionOverrides() {
  MotorStruct joints[octopus_board::JOINT_COUNT] = {};
  for (MotorStruct& joint : joints) {
    joint.direction_reversed = 9;
  }

  octopus_board::Apply_Joint_Direction_Overrides(joints);
  const std::array<int, 6> expected = {{0, 0, 1, 1, 1, 1}};
  for (std::size_t joint = 0; joint < expected.size(); ++joint) {
    assert(joints[joint].direction_reversed == expected.at(joint));
  }
}

void testGPIOAndTMCInitialization() {
  fake_arduino::reset();
  fake_arduino::setInput(ESTOP, LOW);

  Init_Digital_Outputs();
  assertMappedEnableLevels(HIGH);
  assertEveryStepLow();
  assertEveryStepMode(OUTPUT);
  assertEveryChipSelectHigh();
  for (const std::uint32_t pin : kDirectionBySlot) {
    assert(fake_arduino::modes.at(pin) == OUTPUT);
  }
  assert(OUTPUT1 == FAN0);
  assert(fake_arduino::modes.at(FAN0) == OUTPUT);
  assert(fake_arduino::levels.at(FAN0) == LOW);
  assert(fake_arduino::levels.at(FAN4) == HIGH);
  assert(fake_arduino::levels.at(FAN5) == HIGH);

  Init_Digital_Inputs();
  assert(fake_arduino::modes.at(ESTOP) == INPUT_PULLUP);
  assert(fake_arduino::interrupt_modes.at(ESTOP) == RISING);
  assert(fake_arduino::callbacks.at(ESTOP) != nullptr);

  TMC5160Stepper drivers[octopus_board::JOINT_COUNT] = {};
  for (const std::uint32_t pin : kChipSelectBySlot) {
    fake_arduino::levels.at(pin) = LOW;
  }
  fake_arduino::levels.at(FLASH_SELECT) = LOW;
  octopus_board::Prepare_TMC_SPI(drivers);
  assertEveryChipSelectHigh();
  for (const TMC5160Stepper& driver : drivers) {
    assert(driver.spiSpeed() == octopus_board::TMC_SPI_HZ);
    assert(driver.setSPISpeedCalls() == 1);
  }

  octopus_board::Finish_Driver_Initialization();
  assertMappedEnableLevels(LOW);
  assert(!octopus_board::STOP7_Motion_Locked());
}

void testStop7HoldResetResumeLifecycle() {
  AccelStepper steppers[octopus_board::JOINT_COUNT] = {};
  MotorStruct joints[octopus_board::JOINT_COUNT] = {};
  for (int joint = 0; joint < octopus_board::JOINT_COUNT; ++joint) {
    steppers[joint].setCurrentPosition(1000L + joint);
    steppers[joint].setSpeed(200.0F + joint);
    steppers[joint].moveTo(2000L + joint);
    joints[joint].homing_stage_1 = 1;
    joints[joint].homing_stage_2 = 1;
    joints[joint].homed = 1;
  }
  Robot robot = {};
  int home_command = 1;

  for (const std::uint32_t pin : kStepBySlot) {
    fake_arduino::levels.at(pin) = HIGH;
  }
  fake_arduino::setInput(ESTOP, HIGH);
  fake_arduino::fireInterrupt(ESTOP);

  // The RISING callback kills every STEP immediately and leaves the mapped
  // active-low enables untouched to retain holding torque.
  assertEveryStepLow();
  assertEveryStepMode(INPUT_PULLDOWN);
  assertMappedEnableLevels(LOW);

  // AccelStepper may still call digitalWrite later in the interrupted
  // upstream loop. The GPIO mode must remain input-pulldown until recovery.
  for (const std::uint32_t pin : kStepBySlot) {
    digitalWrite(pin, HIGH);
  }
  assertEveryStepMode(INPUT_PULLDOWN);

  octopus_board::Poll_STOP7(steppers, joints, &robot, &home_command);
  assert(octopus_board::STOP7_Motion_Locked());
  assert(!octopus_board::STOP7_Reset_Acknowledged());
  assert(robot.disabled == 1);
  assert(home_command == 0);
  assertMappedEnableLevels(LOW);
  for (int joint = 0; joint < octopus_board::JOINT_COUNT; ++joint) {
    assert(steppers[joint].speed() == 0.0F);
    assert(steppers[joint].targetPosition() ==
           steppers[joint].currentPosition());
    assert(steppers[joint].setSpeedCalls() == 2);
    assert(steppers[joint].moveToCalls() == 2);
    assert(joints[joint].homing_stage_1 == 0);
    assert(joints[joint].homing_stage_2 == 0);
    assert(joints[joint].homed == 0);
  }

  // Reclosing or an ENABLE by itself must not resume.
  fake_arduino::setInput(ESTOP, LOW);
  octopus_board::Poll_STOP7(steppers, joints, &robot, &home_command);
  octopus_board::Record_Valid_Command_Frame(101);
  octopus_board::Commit_STOP7_Recovery(101);
  assert(octopus_board::STOP7_Motion_Locked());
  assertMappedEnableLevels(LOW);

  // A reset seen while the loop is open is discarded.
  octopus_board::Record_Valid_Command_Frame(103);
  fake_arduino::setInput(ESTOP, HIGH);
  octopus_board::Record_Valid_Command_Frame(101);
  fake_arduino::setInput(ESTOP, LOW);
  octopus_board::Record_Valid_Command_Frame(101);
  octopus_board::Commit_STOP7_Recovery(101);
  assert(octopus_board::STOP7_Motion_Locked());

  // A valid pair that is overwritten by a later command cannot unlock.
  octopus_board::Record_Valid_Command_Frame(103);
  octopus_board::Record_Valid_Command_Frame(101);
  octopus_board::Record_Valid_Command_Frame(69);
  octopus_board::Commit_STOP7_Recovery(69);
  assert(octopus_board::STOP7_Motion_Locked());

  // Waldo's reset edge keeps the acknowledgement across IDLE frames.
  octopus_board::Record_Valid_Command_Frame(255);
  octopus_board::Record_Valid_Command_Frame(103);
  assert(octopus_board::STOP7_Reset_Acknowledged());
  octopus_board::Record_Valid_Command_Frame(255);
  assert(octopus_board::STOP7_Reset_Acknowledged());

  // Separate explicit ENABLE commits the acknowledged recovery.
  octopus_board::Record_Valid_Command_Frame(101);
  assert(octopus_board::STOP7_Motion_Locked());
  octopus_board::Commit_STOP7_Recovery(101);
  assert(!octopus_board::STOP7_Motion_Locked());
  assert(octopus_board::STOP7_Reset_Acknowledged());
  assertEveryStepMode(OUTPUT);
  assertEveryStepLow();
  assertMappedEnableLevels(LOW);
}

void testFinishInitializationRollsBackStopDuringEnable() {
  AccelStepper steppers[octopus_board::JOINT_COUNT] = {};
  MotorStruct joints[octopus_board::JOINT_COUNT] = {};
  Robot robot = {};
  int home_command = 0;

  octopus_board::Set_Mapped_Drivers_Enabled(false);
  fake_arduino::setInput(ESTOP, LOW);
  fake_arduino::interrupt_on_low_write_pin = PD4;
  octopus_board::Finish_Driver_Initialization();

  assert(octopus_board::STOP7_Motion_Locked());
  assertEveryStepMode(INPUT_PULLDOWN);
  assertMappedEnableLevels(HIGH);

  fake_arduino::setInput(ESTOP, LOW);
  octopus_board::Poll_STOP7(steppers, joints, &robot, &home_command);
  octopus_board::Record_Valid_Command_Frame(103);
  octopus_board::Record_Valid_Command_Frame(255);
  octopus_board::Record_Valid_Command_Frame(101);
  octopus_board::Commit_STOP7_Recovery(101);
  assert(!octopus_board::STOP7_Motion_Locked());
  assertMappedEnableLevels(LOW);
}

void testStopOutsideHomePreservesHomedState() {
  AccelStepper steppers[octopus_board::JOINT_COUNT] = {};
  MotorStruct joints[octopus_board::JOINT_COUNT] = {};
  Robot robot = {};
  int home_command = 0;
  for (MotorStruct& joint : joints) {
    joint.homed = 1;
  }

  fake_arduino::setInput(ESTOP, HIGH);
  fake_arduino::fireInterrupt(ESTOP);
  octopus_board::Poll_STOP7(steppers, joints, &robot, &home_command);
  for (const MotorStruct& joint : joints) {
    assert(joint.homed == 1);
  }

  fake_arduino::setInput(ESTOP, LOW);
  octopus_board::Poll_STOP7(steppers, joints, &robot, &home_command);
  octopus_board::Record_Valid_Command_Frame(103);
  octopus_board::Record_Valid_Command_Frame(255);
  octopus_board::Record_Valid_Command_Frame(101);
  octopus_board::Commit_STOP7_Recovery(101);
  assert(!octopus_board::STOP7_Motion_Locked());
}

void testOpenAtBootStyleFinishRequiresResetResume() {
  fake_arduino::setInput(ESTOP, HIGH);
  octopus_board::Finish_Driver_Initialization();
  assert(octopus_board::STOP7_Motion_Locked());
  assert(!octopus_board::STOP7_Reset_Acknowledged());
  assertMappedEnableLevels(HIGH);

  fake_arduino::setInput(ESTOP, LOW);
  octopus_board::Record_Valid_Command_Frame(101);
  octopus_board::Commit_STOP7_Recovery(101);
  assert(octopus_board::STOP7_Motion_Locked());
  assertMappedEnableLevels(HIGH);
  octopus_board::Record_Valid_Command_Frame(103);
  octopus_board::Record_Valid_Command_Frame(255);
  assert(octopus_board::STOP7_Reset_Acknowledged());
  octopus_board::Record_Valid_Command_Frame(101);
  fake_arduino::interrupt_on_low_write_pin = PD4;
  octopus_board::Commit_STOP7_Recovery(101);
  assert(octopus_board::STOP7_Motion_Locked());
  assertEveryStepMode(INPUT_PULLDOWN);
  assertMappedEnableLevels(HIGH);

  // A fresh reset is required after the edge injected during restore.
  fake_arduino::setInput(ESTOP, LOW);
  AccelStepper steppers[octopus_board::JOINT_COUNT] = {};
  MotorStruct joints[octopus_board::JOINT_COUNT] = {};
  Robot robot = {};
  int home_command = 0;
  octopus_board::Poll_STOP7(steppers, joints, &robot, &home_command);
  octopus_board::Record_Valid_Command_Frame(103);
  octopus_board::Record_Valid_Command_Frame(255);
  octopus_board::Record_Valid_Command_Frame(101);
  octopus_board::Commit_STOP7_Recovery(101);
  assert(!octopus_board::STOP7_Motion_Locked());
  assertEveryStepMode(OUTPUT);
  assertMappedEnableLevels(LOW);
}

}  // namespace

int main() {
  testLiteralJointPinMap();
  testDirectionOverrides();
  testGPIOAndTMCInitialization();
  testFinishInitializationRollsBackStopDuringEnable();
  testStop7HoldResetResumeLifecycle();
  testStopOutsideHomePreservesHomedState();
  testOpenAtBootStyleFinishRequiresResetResume();
  std::cout << "Exp23 octopus_board native tests: PASS\n";
  return 0;
}
