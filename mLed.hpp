#pragma once
#include <Arduino.h>

#define LED_POWER       (0)
#define LED_LEVEL_ONE   (8)
#define LED_LEVEL_TWO   (7)
#define LED_LEVEL_THREE (6)
#define LED_LEVEL_FOUR  (5)
#define LED_LEVEL_FIVE  (4)
#define LED_LEVEL_SIX   (3)
#define LED_LEVEL_SEVEN (2)

enum class LedLevel : unsigned char
{
  LEVEL_ONE = 1,
  LEVEL_TWO,
  LEVEL_THREE,
  LEVEL_FOUR,
  LEVEL_FIVE,
  LEVEL_SIX,
  LEVEL_SEVEN,
};


class mLed
{
public:
  mLed(const uint8_t pin) : _pin(pin) { pinMode(this->_pin, OUTPUT); disable(); };
  void enable(void) { digitalWrite(this->_pin, LOW); };
  void disable(void) { digitalWrite(this->_pin, HIGH); }
private:
  const uint8_t _pin;
};

class mLedPower : public mLed
{
public:
  mLedPower() : mLed(LED_POWER) {};
};

class mLedLevel
{
public:
  mLedLevel()
  {
    this->_leds[0] = &this->_level_1;
    this->_leds[1] = &this->_level_2;
    this->_leds[2] = &this->_level_3;
    this->_leds[3] = &this->_level_4;
    this->_leds[4] = &this->_level_5;
    this->_leds[5] = &this->_level_6;
    this->_leds[6] = &this->_level_7;
  };
  
  void display(LedLevel level);
  void disableAll(void);
private:
  mLed _level_1 = mLed(LED_LEVEL_ONE);
  mLed _level_2 = mLed(LED_LEVEL_TWO);
  mLed _level_3 = mLed(LED_LEVEL_THREE);
  mLed _level_4 = mLed(LED_LEVEL_FOUR);
  mLed _level_5 = mLed(LED_LEVEL_FIVE);
  mLed _level_6 = mLed(LED_LEVEL_SIX);
  mLed _level_7 = mLed(LED_LEVEL_SEVEN);

  mLed* _leds[7];

  const unsigned char _countLeds = 7;
};
