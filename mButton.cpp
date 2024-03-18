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
    if(this->_counter > this->_countLongPressed) // Держат больше 6с
    { 
      this->_state = mButtonState::LONG_PRESSED;
      this->_counter = this->_countLongPressed + 1; // Чтобы не было переполнения
    }
  } else
  {
    if(this->_counter >= this->_countShortPressed) // Прошло больше 50 мс
    {
      this->_state = mButtonState::SHORT_PRESSED;
      
      if(this->_counter > this->_countLongPressed) // Держат больше 6с
      { 
        this->_state = mButtonState::LONG_PRESSED;
        this->_counter = this->_countLongPressed + 1; // Чтобы не было переполнения
      }
    } else 
      this->_state = mButtonState::NO_PRESSED;

    this->_counter = 0;
  }
}

bool mButton::buttonStateIsChange(void)
{
    if (mButtonState state = this->getState(); (this->_preState != state))
    {
      this->_preState = state;
      if(state != mButtonState::NO_PRESSED)
        return true;
    }
    return false;
}
