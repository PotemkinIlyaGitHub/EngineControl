#include <avr/io.h>
#include <avr/interrupt.h>
#include "mEngineControl.hpp"
#include "mEeprom.hpp"
#include "mDebug.hpp"

#define ADDRESS_ENGINE_WORK_MODE (4)



mEngineControl::mEngineControl(const int leftRotationPin,
                 const int rightRotationPin,
                 const int pwmPin): _leftRotationPin(leftRotationPin),
                                    _rightRotationPin(rightRotationPin),
                                    _pwmPin(pwmPin)
{

}

void mEngineControl::init(void)
{
    // Увеличим частоту шим
    TCCR2B = 0b00000010;  // x8
    TCCR2A = 0b00000011;  // fast pwm
    pinMode (this->_rightRotationPin, OUTPUT);
    pinMode (this->_leftRotationPin, OUTPUT);
    pinMode (this->_pwmPin, OUTPUT);
    mEeprom eeprom;
    eeprom.read(ADDRESS_ENGINE_WORK_MODE, &this->_currentWorkPeriod);

    mDebug debug;
    debug.print("Инициализация engineControl");
    debug.print("Режим работы: ");
    debug.print((uint8_t) this->_currentWorkPeriod);
}

void mEngineControl::start()
{
  if(this->_state != mEngineControl::engineState::STOP)
    return;
  mDebug debug;
  this->_tick = 0;
  this->_timePeriod = 500 * (uint8_t )this->_currentWorkPeriod / 10;
  this->_state = mEngineControl::engineState::START;
  debug.print("Start");
}

void mEngineControl::stop()
{
  digitalWrite(this->_leftRotationPin, LOW);
  digitalWrite(this->_rightRotationPin, LOW);
  analogWrite(this->_pwmPin, 0);
}

void mEngineControl::setLeftRotation()
{
  digitalWrite(this->_leftRotationPin, HIGH);
  digitalWrite(this->_rightRotationPin, LOW);
  analogWrite(this->_pwmPin, this->_speed);
}

void mEngineControl::setRightRotation()
{
  digitalWrite(this->_leftRotationPin, LOW);
  digitalWrite(this->_rightRotationPin, HIGH);
  analogWrite(this->_pwmPin, this->_speed);
}

// Запускает ход основной работы
void mEngineControl::procedure(void)
{ 
  this->_tick++;

  switch(this->_state)
  {
    case mEngineControl::engineState::STOP :
    { 
      stop();
      return;
    }
    case mEngineControl::engineState::START :
    {
      this->_tick = 0;
      setRightRotation();
      this->_state = mEngineControl::engineState::RIGHT_ROTATION;
      return;
    }
    case mEngineControl::engineState::RIGHT_ROTATION :
    {
      if(this->_tick < this->_timePeriod)
        return;
      this->_tick = 0;
      stop();
      this->_state = mEngineControl::engineState::PREPEAR_LEFT_ROTATION;
      return;
    }
    case mEngineControl::engineState::LEFT_ROTATION :
    {
      if(this->_tick < this->_timePeriod)
        return;
      this->_tick = 0;
      this->_state = mEngineControl::engineState::STOP;
      stop();
      return;
    }
    // case mEngineControl::engineState::PREPEAR_RIGHT_ROTATION :
    // {
    //   if(this->_tick < 20) // Не прошло 200мс
    //   { 
    //     stop();
    //     return;
    //   }
    //   this->_tick = 0;

    //   setRightRotation();
    //   start();

    //   this->_state = mEngineControl::engineState::RIGHT_ROTATION;
    //   return;
    // }
    case mEngineControl::engineState::PREPEAR_LEFT_ROTATION :
    {
      if(this->_tick < 20) // Не прошло 20мс
      { 
        stop();
        return;
      }
      this->_tick = 0;
      setLeftRotation();
      this->_state = mEngineControl::engineState::LEFT_ROTATION;
      return;
    }
    default:
    {
      stop();
      this->_state = mEngineControl::engineState::STOP;
    }
  }
}

mWorkPeriod mEngineControl::getNextWorkPeriod(mWorkPeriod p)
{
  switch (p)
  {
    case mWorkPeriod::ONE :
      return mWorkPeriod::TWO;
    case mWorkPeriod::TWO :
      return mWorkPeriod::THREE;
    case mWorkPeriod::THREE :
      return mWorkPeriod::FOUR;
    case mWorkPeriod::FOUR :
      return mWorkPeriod::FIVE;
    case mWorkPeriod::FIVE :
      return mWorkPeriod::SIX;
    case mWorkPeriod::SIX :
      return mWorkPeriod::SEVEN;
    case mWorkPeriod::SEVEN :
      return mWorkPeriod::EIGHT;
    case mWorkPeriod::EIGHT :
      return mWorkPeriod::NINE;
    case mWorkPeriod::NINE :
      return mWorkPeriod::TEN;
    case mWorkPeriod::TEN :
      return mWorkPeriod::TEN;
    default :
      return mWorkPeriod::ONE;
  }
}
mWorkPeriod mEngineControl::getPreviousWorkPeriod(mWorkPeriod p)
{
  switch (p)
  {
    case mWorkPeriod::ONE :
      return mWorkPeriod::ONE;
    case mWorkPeriod::TWO :
      return mWorkPeriod::ONE;
    case mWorkPeriod::THREE :
      return mWorkPeriod::TWO;
    case mWorkPeriod::FOUR :
      return mWorkPeriod::THREE;
    case mWorkPeriod::FIVE :
      return mWorkPeriod::FOUR;
    case mWorkPeriod::SIX :
      return mWorkPeriod::FIVE;
    case mWorkPeriod::SEVEN :
      return mWorkPeriod::SIX;
    case mWorkPeriod::EIGHT :
      return mWorkPeriod::SEVEN;
    case mWorkPeriod::NINE :
      return mWorkPeriod::EIGHT;
    case mWorkPeriod::TEN :
      return mWorkPeriod::NINE;
    default :
      return mWorkPeriod::ONE;
  }
} 

void mEngineControl::upPeriodCycle()
{
  mEeprom eeprom;
  mDebug debug;
  auto period = mWorkPeriod::ONE;
  eeprom.read(ADDRESS_ENGINE_WORK_MODE, &period);

  if(period == mWorkPeriod::TEN)
    return;
  
  period = this->getNextWorkPeriod(period);
  this->_currentWorkPeriod = period;
  eeprom.write(ADDRESS_ENGINE_WORK_MODE, period);

  debug.print("Режим изменен на ");
  debug.print((uint8_t)period);
}

void mEngineControl::downPeriodCycle()
{
  mEeprom eeprom;
  mDebug debug;
  auto period = mWorkPeriod::ONE;
  eeprom.read(ADDRESS_ENGINE_WORK_MODE, &period);

  if(period == mWorkPeriod::ONE)
    return;
  
  period = this->getPreviousWorkPeriod(period);
  this->_currentWorkPeriod = period;
  eeprom.write(ADDRESS_ENGINE_WORK_MODE, period);


  debug.print("Режим изменен на ");
  debug.print((uint8_t)period);
}
