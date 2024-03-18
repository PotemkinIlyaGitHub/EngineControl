#include <stdint.h>
#pragma once
#include <Arduino.h>

enum class mButtonState
{
  NO_PRESSED = 0,
  SHORT_PRESSED,
  LONG_PRESSED
};

enum class mButtonPinState
{
  PIN_STATE_LOW = 0,
  PIN_STATE_HIGHT 
};


class mButton
{
public:
  mButton(const uint8_t pin);
  mButtonState getState(void) { return this->_state; }
  inline void setState(mButtonState state) { this->_state = state; }
  inline void init(void) { pinMode(this->_pin, INPUT_PULLUP); }
  mButtonPinState getPinState(void);

  void checkState(void);
  bool buttonStateIsChange(void);

private:
  const uint8_t _pin;
  uint16_t _counter = 0; // Счетчик для определения нажатия
  const uint16_t _countShortPressed = 5; // Количество тиков таймера для определения короткого нажатия
  const uint16_t _countLongPressed = 600;// Количество тиков таймера для определения длинного нажатия
  mButtonState _state = mButtonState::NO_PRESSED;
  mButtonState _preState = mButtonState::NO_PRESSED;
};