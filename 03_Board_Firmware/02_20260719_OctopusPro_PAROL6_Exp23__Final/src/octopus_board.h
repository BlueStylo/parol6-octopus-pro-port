#ifndef OCTOPUS_BOARD_H
#define OCTOPUS_BOARD_H

#include <AccelStepper.h>
#include <TMCStepper.h>

#include "structs.h"

namespace octopus_board
{

constexpr int JOINT_COUNT = 6;
constexpr int DRIVER_SLOT_COUNT = 8;
constexpr unsigned long TMC_SPI_HZ = 1000000UL;

// Applies only the board/wiring polarity correction required to let the
// untouched upstream HOME signs produce the already validated physical DIR
// levels. All current, ratio, offsets, limits and trigger levels stay upstream.
void Apply_Joint_Direction_Overrides(MotorStruct (&joints)[JOINT_COUNT]);

// Octopus has one active-low EN and one CS per socket instead of the original
// board's single GLOBAL_ENABLE line.
void Set_Mapped_Drivers_Enabled(bool enabled);
void Prepare_TMC_SPI(TMC5160Stepper (&drivers)[JOINT_COUNT]);
void Idle_All_Chip_Selects();
void Finish_Driver_Initialization();

// STOP7 is the one intentional non-upstream policy: an open NC loop latches
// motion stopped while preserving already-active holding torque. Reclosing
// alone does not resume; newly received command 103 then 101 is required.
void Poll_STOP7(AccelStepper (&steppers)[JOINT_COUNT],
                MotorStruct (&joints)[JOINT_COUNT],
                Robot *robot,
                int *home_command);
void Enforce_STOP7_Hold(AccelStepper (&steppers)[JOINT_COUNT],
                        MotorStruct (&joints)[JOINT_COUNT],
                        Robot *robot,
                        int *home_command);
void Record_Valid_Command_Frame(unsigned char command);
void Commit_STOP7_Recovery(unsigned char current_command);
bool STOP7_Motion_Locked();
bool STOP7_Reset_Acknowledged();

} // namespace octopus_board

#endif
