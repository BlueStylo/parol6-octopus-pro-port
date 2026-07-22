#ifndef IODEFS_H
#define IODEFS_H

#include <Arduino.h>
#include <stdio.h>

/*
 * Exp23 keeps the upstream PAROL6 driver[]/stepper[] construction unchanged.
 * These compatibility symbols therefore follow the upstream's intentionally
 * non-linear joint order:
 *   J1=1, J2=6, J3=5, J4=4, J5=2, J6=3.
 *
 * Physical Octopus map:
 *   J1..J6 = MOTOR6/MOTOR5/MOTOR4/MOTOR3/MOTOR1/MOTOR0.
 * MOTOR2 is the parallel Z output and remains bypassed; MOTOR7 is spare.
 */

// J1 -> MOTOR6
#define PUL1 PE2
#define DIR1 PE3
#define SELECT1 PE1
#define ENABLE1 PD4

// J5 -> MOTOR1
#define PUL2 PG0
#define DIR2 PG1
#define SELECT2 PD11
#define ENABLE2 PF15

// J6 -> MOTOR0
#define PUL3 PF13
#define DIR3 PF12
#define SELECT3 PC4
#define ENABLE3 PF14

// J4 -> MOTOR3
#define PUL4 PG4
#define DIR4 PC1
#define SELECT4 PC7
#define ENABLE4 PA0

// J3 -> MOTOR4
#define PUL5 PF9
#define DIR5 PF10
#define SELECT5 PF2
#define ENABLE5 PG2

// J2 -> MOTOR5
#define PUL6 PC13
#define DIR6 PF0
#define SELECT6 PE4
#define ENABLE6 PF1

// Preserve upstream Joint[].LIMIT assignments while routing final sensors.
#define LIMIT1 PG13 // J6 / STOP5
#define LIMIT2 PG10 // J2 / STOP2
#define LIMIT3 PG9  // J3 / STOP1
#define LIMIT4 PG12 // J4 / STOP4
#define LIMIT5 PG6  // J5 / STOP0
#define LIMIT6 PG11 // J1 / STOP3

// Physical slots intentionally not assigned to a joint.
#define UNUSED_MOTOR2_STEP PF11
#define UNUSED_MOTOR2_DIR PG3
#define UNUSED_MOTOR2_ENABLE PG5
#define UNUSED_MOTOR2_SELECT PC6
#define UNUSED_MOTOR7_STEP PE6
#define UNUSED_MOTOR7_DIR PA14
#define UNUSED_MOTOR7_ENABLE PE0
#define UNUSED_MOTOR7_SELECT PD3

#define MISO PA6
#define MOSI PA7
#define SCK PA5
#define FLASH_SELECT PF8

#define FAN0 PA8
#define FAN4 PD14
#define FAN5 PD15

// The original power-latch circuit is absent. These aliases are retained only
// so the byte-identical upstream utility source compiles; Exp23 never calls it.
#define SUPPLY_ON_OFF PC10
#define SUPPLY_BUTTON_STATE PC14

// Preserve the original OUTPUT1 packet bit and Waldo pneumatic-tool contract,
// but route its load through FAN0's active-HIGH MOSFET power stage. OUTPUT2
// remains a logic-level spare on EXP1.
#define INPUT1 PE12
#define INPUT2 PE13
#define OUTPUT1 FAN0
#define OUTPUT2 PE15

#define ESTOP PG15 // STOP7: NC loop closed LOW, open/wire break HIGH

#define VBUS PB0 // Original ADC source retained but not linked on Octopus.

#define USB_D_PLUS PA12
#define USB_D_MINUS PA11 

#define CAN1TX PD1
#define CAN1RX PD0

#endif
