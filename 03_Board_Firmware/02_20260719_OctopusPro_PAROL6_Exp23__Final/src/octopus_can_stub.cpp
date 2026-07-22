#include "coms_CAN.h"

// The Octopus CAN transceiver/gripper wiring has not yet been commissioned.
// Keep the upstream 56/60-byte gripper fields and call graph intact without
// touching unverified CAN pins or allowing Setup_CAN_bus() to block startup.

void Extract_from_CAN_ID(unsigned int)
{
}

unsigned int Combine_2_CAN_ID(unsigned int Node_ID,
                             unsigned int Command_ID,
                             bool errorBit)
{
  unsigned int canId = 0;
  canId |= (Node_ID & 0xF) << 7;
  canId |= (Command_ID & 0x3F) << 1;
  canId |= (errorBit & 0x1);
  return canId;
}

void Setup_CAN_bus()
{
}

void CAN_protocol(Stream &)
{
}

void Send_gripper_cal()
{
}

void Send_gripper_pack()
{
}

void Send_clear_error()
{
}

void Send_gripper_pack_empty()
{
}
