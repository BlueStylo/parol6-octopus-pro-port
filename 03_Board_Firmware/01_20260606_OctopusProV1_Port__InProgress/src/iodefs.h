#ifndef IODEFS_H
#define IODEFS_H

#include <Arduino.h>
#include <stdio.h>

/*
 * BIGTREETECH Octopus Pro V1.0, STM32F446ZET6.
 *
 * PAROL6's original code does not map PUL1..PUL6 directly to J1..J6:
 * J1=PUL1, J2=PUL6, J3=PUL5, J4=PUL4, J5=PUL2, J6=PUL3.
 * The definitions below keep that software mapping while assigning
 * J1..J6 to Octopus MOTOR0..MOTOR5 in physical order.
 */

// J1 -> MOTOR0
#define PUL1 PF13
#define DIR1 PF12
#define SELECT1 PC4
#define ENABLE1 PF14

// J5 -> MOTOR4
#define PUL2 PF9
#define DIR2 PF10
#define SELECT2 PF2
#define ENABLE2 PG2

// J6 -> MOTOR5
#define PUL3 PC13
#define DIR3 PF0
#define SELECT3 PE4
#define ENABLE3 PF1

// J4 -> MOTOR3
#define PUL4 PG4
#define DIR4 PC1
#define SELECT4 PC7
#define ENABLE4 PA0

// J3 -> MOTOR2
#define PUL5 PF11
#define DIR5 PG3
#define SELECT5 PC6
#define ENABLE5 PG5

// J2 -> MOTOR1
#define PUL6 PG0
#define DIR6 PG1
#define SELECT6 PD11
#define ENABLE6 PF15

// Endstop/DIAG header signal pins. Remove driver DIAG jumpers when using wired sensors.
#define LIMIT1 PG13
#define LIMIT2 PG9
#define LIMIT3 PG10
#define LIMIT4 PG11
#define LIMIT5 PG12
#define LIMIT6 PG6

#define MISO PA6
#define MOSI PA7
#define SCK PA5
#define FLASH_SELECT PA4

// Non-motion I/O placeholders on EXP headers. Verify before wiring external devices.
#define LED1 PE9
#define LED2 PE10

#define SUPPLY_ON_OFF PE8
#define SUPPLY_BUTTON_STATE PE7

#define INPUT1 PE12
#define INPUT2 PE13

#define OUTPUT1 PE14
#define OUTPUT2 PE15

#define ESTOP PG15

#define VBUS PB0  // ADC1 CHANNEL 8

#define USB_D_PLUS  PA12 
#define USB_D_MINUS PA11 

static inline void Init_Driver_Enable_Pins(void)
{
  pinMode(ENABLE1, OUTPUT);
  pinMode(ENABLE2, OUTPUT);
  pinMode(ENABLE3, OUTPUT);
  pinMode(ENABLE4, OUTPUT);
  pinMode(ENABLE5, OUTPUT);
  pinMode(ENABLE6, OUTPUT);
}

static inline void Write_Driver_Enable_Pins(uint8_t state)
{
  digitalWrite(ENABLE1, state);
  digitalWrite(ENABLE2, state);
  digitalWrite(ENABLE3, state);
  digitalWrite(ENABLE4, state);
  digitalWrite(ENABLE5, state);
  digitalWrite(ENABLE6, state);
}

#endif
