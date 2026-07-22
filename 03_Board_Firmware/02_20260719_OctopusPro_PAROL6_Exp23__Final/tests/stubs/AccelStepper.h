#pragma once

#include <cstdint>

class AccelStepper {
 public:
  static constexpr std::uint8_t DRIVER = 1U;

  AccelStepper() = default;
  AccelStepper(std::uint8_t, std::uint8_t, std::uint8_t,
               std::uint8_t = 0U, std::uint8_t = 0U, bool = true) {}

  void setCurrentPosition(long position) {
    current_position_ = position;
    target_position_ = position;
  }

  long currentPosition() const { return current_position_; }
  long targetPosition() const { return target_position_; }

  void setSpeed(float speed) {
    speed_ = speed;
    ++set_speed_calls_;
  }

  float speed() const { return speed_; }
  int setSpeedCalls() const { return set_speed_calls_; }

  void moveTo(long position) {
    target_position_ = position;
    ++move_to_calls_;
  }

  int moveToCalls() const { return move_to_calls_; }

 private:
  long current_position_ = 0L;
  long target_position_ = 0L;
  float speed_ = 0.0F;
  int set_speed_calls_ = 0;
  int move_to_calls_ = 0;
};
