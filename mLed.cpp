#include "mLed.hpp"

void mLedLevel::display(LedLevel level)
{
  for(unsigned char i = 0; i < this->_countLeds; i++)
  {
    if(i < (unsigned char)level)
      this->_leds[i]->enable();
    else
    this->_leds[i]->disable();
  }
}
