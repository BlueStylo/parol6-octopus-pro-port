#pragma once

// Suppress Exp23's full upstream structs.h after providing only the fields
// touched by octopus_board.cpp. This keeps the adapter unit test independent
// of unrelated protocol and gripper data.
#ifndef STRUCTS
#define STRUCTS
#endif

struct MotorStruct {
  int direction_reversed = 0;
  int homing_stage_1 = 0;
  int homing_stage_2 = 0;
  int homed = 0;
};

struct Robot {
  int disabled = 0;
};
