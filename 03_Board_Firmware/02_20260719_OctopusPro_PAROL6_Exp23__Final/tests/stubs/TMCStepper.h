#pragma once

#include <cstdint>

class TMC5160Stepper {
 public:
  TMC5160Stepper() = default;

  void setSPISpeed(std::uint32_t speed) {
    spi_speed_ = speed;
    ++set_spi_speed_calls_;
  }

  std::uint32_t spiSpeed() const { return spi_speed_; }
  int setSPISpeedCalls() const { return set_spi_speed_calls_; }

 private:
  std::uint32_t spi_speed_ = 0U;
  int set_spi_speed_calls_ = 0;
};
