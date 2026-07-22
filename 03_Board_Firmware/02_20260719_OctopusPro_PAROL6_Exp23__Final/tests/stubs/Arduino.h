#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#define LOW 0U
#define HIGH 1U
#define INPUT_PULLUP 2U
#define OUTPUT 3U
#define INPUT_PULLDOWN 4U
#define RISING 5U

// Only the STM32 symbols referenced by Exp23's iodefs.h and
// octopus_board.cpp are needed by the native test.
enum FakePin : std::uint32_t {
  PA0 = 0,
  PA5,
  PA6,
  PA7,
  PA8,
  PA11,
  PA12,
  PA14,
  PB0,
  PC1,
  PC4,
  PC6,
  PC7,
  PC10,
  PC13,
  PC14,
  PD0,
  PD1,
  PD3,
  PD4,
  PD11,
  PD14,
  PD15,
  PE0,
  PE1,
  PE2,
  PE3,
  PE4,
  PE6,
  PE12,
  PE13,
  PE14,
  PE15,
  PF0,
  PF1,
  PF2,
  PF8,
  PF9,
  PF10,
  PF11,
  PF12,
  PF13,
  PF14,
  PF15,
  PG0,
  PG1,
  PG2,
  PG3,
  PG4,
  PG5,
  PG6,
  PG9,
  PG10,
  PG11,
  PG12,
  PG13,
  PG14,
  PG15,
  kFakePinCount,
};

namespace fake_arduino {

using InterruptCallback = void (*)();

extern std::array<std::uint8_t, kFakePinCount> levels;
extern std::array<std::uint8_t, kFakePinCount> modes;
extern std::array<InterruptCallback, kFakePinCount> callbacks;
extern std::array<int, kFakePinCount> interrupt_modes;
extern bool interrupts_enabled;
extern std::uint32_t interrupt_on_low_write_pin;

void reset();
void setInput(std::uint32_t pin, std::uint8_t level);
void fireInterrupt(std::uint32_t pin);

}  // namespace fake_arduino

inline void digitalWrite(std::uint32_t pin, std::uint8_t level) {
  fake_arduino::levels.at(pin) = level;
  if (level == LOW &&
      fake_arduino::interrupt_on_low_write_pin == pin) {
    fake_arduino::interrupt_on_low_write_pin = kFakePinCount;
    fake_arduino::levels.at(PG15) = HIGH;
    const auto callback = fake_arduino::callbacks.at(PG15);
    if (callback != nullptr) {
      callback();
    }
  }
}

inline int digitalRead(std::uint32_t pin) {
  return fake_arduino::levels.at(pin);
}

inline void pinMode(std::uint32_t pin, std::uint8_t mode) {
  fake_arduino::modes.at(pin) = mode;
}

inline void attachInterrupt(std::uint32_t pin,
                            fake_arduino::InterruptCallback callback,
                            int mode) {
  fake_arduino::callbacks.at(pin) = callback;
  fake_arduino::interrupt_modes.at(pin) = mode;
}

inline void noInterrupts() {
  fake_arduino::interrupts_enabled = false;
}

inline void interrupts() {
  fake_arduino::interrupts_enabled = true;
}
