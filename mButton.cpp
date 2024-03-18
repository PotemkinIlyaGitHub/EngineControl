#include "mButton.hpp"

mButton::mButton(const uint8_t pin) : _pin(pin)
{
   
};

mButtonPinState mButton::getPinState(void)
{
  return (digitalRead(this->_pin) == 1) ? mButtonPinState::PIN_STATE_LOW : mButtonPinState::PIN_STATE_HIGHT;
}

void mButton::checkState(void)
{
  if(this->getPinState() == mButtonPinState::PIN_STATE_HIGHT)
  {
    ++this->_counter;
    if(this->_counter >= this->_countShortPressed) // Прошло больше 50 мс
    {
      this->_state = mButtonState::SHORT_PRESSED;
      
      if(this->_counter > this->_countLongPressed) // Держат больше 2с
      { 
        this->_state = mButtonState::LONG_PRESSED;
        this->_counter = this->_countLongPressed + 1; // Чтобы не было переполнения
      }
    }
  } else
  {
    this->_counter = 0;
    this->_state = mButtonState::NO_PRESSED;
  }
}

bool mButton::buttonStateIsChange(void)
{
    if (mButtonState state = this->getState(); (this->_preState != state) || (state == mButtonState::LONG_PRESSED))
    {
        this->_preState = state;
        if ((state == mButtonState::SHORT_PRESSED) ||
            (state == mButtonState::LONG_PRESSED && ((millis() % 500) == 0))) // Для определения длительного нажатия
        {
            this->_preState = state;
            return true;
        }
    }
    return false;
}
