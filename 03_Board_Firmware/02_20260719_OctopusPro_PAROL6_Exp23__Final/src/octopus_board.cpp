#include "octopus_board.h"

#include <Arduino.h>

#include "hw_init.h"
#include "iodefs.h"

namespace
{

const uint32_t kStepPins[octopus_board::DRIVER_SLOT_COUNT] = {
    PF13, PG0, PF11, PG4, PF9, PC13, PE2, PE6};
const uint32_t kDirectionPins[octopus_board::DRIVER_SLOT_COUNT] = {
    PF12, PG1, PG3, PC1, PF10, PF0, PE3, PA14};
const uint32_t kEnablePins[octopus_board::DRIVER_SLOT_COUNT] = {
    PF14, PF15, PG5, PA0, PG2, PF1, PD4, PE0};
const uint32_t kChipSelectPins[octopus_board::DRIVER_SLOT_COUNT] = {
    PC4, PD11, PC6, PC7, PF2, PE4, PE1, PD3};

// J1..J6 -> MOTOR6/MOTOR5/MOTOR4/MOTOR3/MOTOR1/MOTOR0.
const uint8_t kMappedMotorSlots[octopus_board::JOINT_COUNT] = {
    6, 5, 4, 3, 1, 0};

volatile bool g_stop7_interrupt_seen = false;
bool g_stop7_latched = false;
bool g_stop7_reset_seen = false;
bool g_stop7_resume_pending = false;
bool g_mapped_drivers_enabled = false;
volatile bool g_step_outputs_gated = false;

void Force_All_Steps_Low()
{
  for (int slot = 0; slot < octopus_board::DRIVER_SLOT_COUNT; ++slot)
  {
    digitalWrite(kStepPins[slot], LOW);
  }
}

void Gate_All_Step_Outputs()
{
  Force_All_Steps_Low();
  for (int slot = 0; slot < octopus_board::DRIVER_SLOT_COUNT; ++slot)
  {
    pinMode(kStepPins[slot], INPUT_PULLDOWN);
  }
  g_step_outputs_gated = true;
}

void STOP7_Interrupt()
{
  Gate_All_Step_Outputs();
  g_stop7_interrupt_seen = true;
}

bool STOP7_Open_Hardware()
{
  return digitalRead(ESTOP) == HIGH;
}

void Latch_STOP7()
{
  g_stop7_reset_seen = false;
  g_stop7_resume_pending = false;
  g_stop7_latched = true;
  Force_All_Steps_Low();
  if (!g_step_outputs_gated)
  {
    Gate_All_Step_Outputs();
  }
}

void Preload_Output(uint32_t pin, uint8_t level)
{
  digitalWrite(pin, level);
  pinMode(pin, OUTPUT);
}

void Restore_Step_Outputs()
{
  for (int slot = 0; slot < octopus_board::DRIVER_SLOT_COUNT; ++slot)
  {
    Preload_Output(kStepPins[slot], LOW);
  }
  g_step_outputs_gated = false;
}

} // namespace

// These two global symbols intentionally replace the original PCB-specific
// hw_init.cpp implementation while keeping upstream main.cpp call sites.
void Init_Digital_Inputs(void)
{
  pinMode(LIMIT1, INPUT_PULLUP);
  pinMode(LIMIT2, INPUT_PULLUP);
  pinMode(LIMIT3, INPUT_PULLUP);
  pinMode(LIMIT4, INPUT_PULLUP);
  pinMode(LIMIT5, INPUT_PULLUP);
  pinMode(LIMIT6, INPUT_PULLUP);
  pinMode(PG14, INPUT_PULLUP); // STOP6 spare
  pinMode(ESTOP, INPUT_PULLUP);

  pinMode(INPUT1, INPUT_PULLUP);
  pinMode(INPUT2, INPUT_PULLUP);

  attachInterrupt(ESTOP, STOP7_Interrupt, RISING);
}

void Init_Digital_Outputs(void)
{
  // Preload all active-low enables inactive before changing GPIO direction.
  for (int slot = 0; slot < octopus_board::DRIVER_SLOT_COUNT; ++slot)
  {
    Preload_Output(kEnablePins[slot], HIGH);
  }

  for (int slot = 0; slot < octopus_board::DRIVER_SLOT_COUNT; ++slot)
  {
    Preload_Output(kStepPins[slot], LOW);
    Preload_Output(kDirectionPins[slot], LOW);
    Preload_Output(kChipSelectPins[slot], HIGH);
  }

  // MAX31865 shares the SPI wires and must never float selected.
  Preload_Output(FLASH_SELECT, HIGH);

  Preload_Output(OUTPUT1, LOW);
  Preload_Output(OUTPUT2, LOW);

  // FAN4/FAN5 are MCU-controlled active-HIGH. FAN6/FAN7 are hard-wired ON
  // whenever V_FAN is present and require no GPIO.
  Preload_Output(FAN4, HIGH);
  Preload_Output(FAN5, HIGH);
}

namespace octopus_board
{

void Apply_Joint_Direction_Overrides(MotorStruct (&joints)[JOINT_COUNT])
{
  // Upstream HOME signs N/N/N/P/N/N then produce the validated physical DIR
  // pin levels LOW/LOW/HIGH/LOW/HIGH/HIGH.
  const bool direction_inverted[JOINT_COUNT] = {
      false, false, true, true, true, true};
  for (int joint = 0; joint < JOINT_COUNT; ++joint)
  {
    joints[joint].direction_reversed = direction_inverted[joint];
  }
}

void Set_Mapped_Drivers_Enabled(bool enabled)
{
  // M2 (parallel Z) and M7 are always disabled.
  digitalWrite(kEnablePins[2], HIGH);
  digitalWrite(kEnablePins[7], HIGH);

  const uint8_t level = enabled ? LOW : HIGH;
  for (int joint = 0; joint < JOINT_COUNT; ++joint)
  {
    digitalWrite(kEnablePins[kMappedMotorSlots[joint]], level);
  }
  g_mapped_drivers_enabled = enabled;
}

void Prepare_TMC_SPI(TMC5160Stepper (&drivers)[JOINT_COUNT])
{
  Idle_All_Chip_Selects();
  for (int joint = 0; joint < JOINT_COUNT; ++joint)
  {
    drivers[joint].setSPISpeed(TMC_SPI_HZ);
  }
}

void Idle_All_Chip_Selects()
{
  for (int slot = 0; slot < DRIVER_SLOT_COUNT; ++slot)
  {
    digitalWrite(kChipSelectPins[slot], HIGH);
  }
  digitalWrite(FLASH_SELECT, HIGH);
}

void Finish_Driver_Initialization()
{
  Idle_All_Chip_Selects();
  if (STOP7_Open_Hardware() || g_stop7_interrupt_seen)
  {
    Latch_STOP7();
    Set_Mapped_Drivers_Enabled(false);
    return;
  }
  Set_Mapped_Drivers_Enabled(true);

  // A STOP edge can arrive in the small interval between the precheck and
  // enabling the mapped sockets. Roll back to the boot-safe EN state if so.
  const bool fault_during_enable =
      STOP7_Open_Hardware() || g_stop7_interrupt_seen;
  if (fault_during_enable)
  {
    Latch_STOP7();
    Set_Mapped_Drivers_Enabled(false);
  }
}

void Enforce_STOP7_Hold(AccelStepper (&steppers)[JOINT_COUNT],
                        MotorStruct (&joints)[JOINT_COUNT],
                        Robot *robot,
                        int *home_command)
{
  if (!g_stop7_latched)
  {
    return;
  }

  const bool home_was_active =
      home_command != nullptr && *home_command != 0;

  Force_All_Steps_Low();
  for (int joint = 0; joint < JOINT_COUNT; ++joint)
  {
    steppers[joint].setSpeed(0);
    steppers[joint].moveTo(steppers[joint].currentPosition());
    if (home_was_active)
    {
      joints[joint].homing_stage_1 = 0;
      joints[joint].homing_stage_2 = 0;
      joints[joint].homed = 0;
    }
  }

  if (robot != nullptr)
  {
    robot->disabled = 1;
  }
  if (home_command != nullptr)
  {
    *home_command = 0;
  }

  // Deliberately do not change any EN pin here. If torque existed when STOP7
  // opened, it stays. If STOP7 was open at boot, EN stays inactive until the
  // explicit 103 -> 101 recovery completes.
}

void Poll_STOP7(AccelStepper (&steppers)[JOINT_COUNT],
                MotorStruct (&joints)[JOINT_COUNT],
                Robot *robot,
                int *home_command)
{
  noInterrupts();
  const bool interrupt_seen = g_stop7_interrupt_seen;
  g_stop7_interrupt_seen = false;
  interrupts();

  if (interrupt_seen || STOP7_Open_Hardware())
  {
    Latch_STOP7();
  }
  Enforce_STOP7_Hold(steppers, joints, robot, home_command);
}

void Record_Valid_Command_Frame(unsigned char command)
{
  if (!g_stop7_latched)
  {
    return;
  }

  if (STOP7_Open_Hardware())
  {
    Latch_STOP7();
    return;
  }

  // Once ENABLE is pending, any later frame means upstream will dispatch that
  // later command instead. Cancel recovery so a batched 103->101->motion
  // sequence cannot bypass the STOP latch.
  if (g_stop7_resume_pending)
  {
    g_stop7_reset_seen = false;
    g_stop7_resume_pending = false;
    return;
  }

  if (command == 103)
  {
    g_stop7_reset_seen = true;
    g_stop7_resume_pending = false;
    return;
  }

  // Waldo sends its reset edge as IDLE(255) -> CLEAR(103) -> IDLE(255).
  // Keep the explicit reset acknowledgement armed across those IDLE frames.
  if (command == 255 && g_stop7_reset_seen)
  {
    return;
  }

  if (command == 101 && g_stop7_reset_seen)
  {
    g_stop7_resume_pending = true;
    return;
  }

  // If a later frame overwrites the pending ENABLE before upstream dispatch,
  // require a fresh CLEAR -> ENABLE pair instead of resuming another command.
  g_stop7_reset_seen = false;
  g_stop7_resume_pending = false;
}

void Commit_STOP7_Recovery(unsigned char current_command)
{
  if (!g_stop7_latched)
  {
    return;
  }

  if (current_command != 101 || !g_stop7_resume_pending)
  {
    return;
  }

  const bool drivers_were_enabled = g_mapped_drivers_enabled;

  // Keep interrupts enabled throughout the restore so any STOP edge is
  // serviced immediately and retained by the sticky ISR flag.
  const bool fault_before_restore =
      STOP7_Open_Hardware() || g_stop7_interrupt_seen;
  if (fault_before_restore)
  {
    Latch_STOP7();
    return;
  }

  Restore_Step_Outputs();
  if (!g_mapped_drivers_enabled)
  {
    Set_Mapped_Drivers_Enabled(true);
  }

  // This check is the recovery linearization point. Any earlier edge was
  // serviced while interrupts were enabled and forces rollback. An edge after
  // this point is a new runtime STOP and the ISR immediately gates STEP again.
  const bool fault_during_restore =
      STOP7_Open_Hardware() || g_stop7_interrupt_seen;
  if (fault_during_restore)
  {
    Gate_All_Step_Outputs();
    if (!drivers_were_enabled)
    {
      Set_Mapped_Drivers_Enabled(false);
    }
    g_stop7_reset_seen = false;
    g_stop7_resume_pending = false;
    return;
  }

  // Clear the latch only after the complete STEP/EN restore succeeds.
  g_stop7_latched = false;
  g_stop7_reset_seen = false;
  g_stop7_resume_pending = false;
}

bool STOP7_Motion_Locked()
{
  return g_stop7_latched;
}

bool STOP7_Reset_Acknowledged()
{
  if (g_stop7_interrupt_seen || STOP7_Open_Hardware())
  {
    return false;
  }
  if (!g_stop7_latched)
  {
    return true;
  }
  return g_stop7_reset_seen;
}

} // namespace octopus_board
