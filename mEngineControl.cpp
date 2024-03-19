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
   // upPwmFrequency();
    pinMode (this->_rightRotationPin, OUTPUT);
    pinMode (this->_leftRotationPin, OUTPUT);
    pinMode (this->_pwmPin, OUTPUT);
    mEeprom eeprom;
    eeprom.read(ADDRESS_ENGINE_WORK_MODE, &this->_currentWorkPeriod);

    mDebug debug;
    debug.print("Инициализация engineControl");
    debug.print("Режим работы: ");
    debug.print((uint8_t) this->_currentWorkPeriod);

    mLedPower ledPower;
    ledPower.enable();

    updateIndications();
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
      return mWorkPeriod::ONE;
    default :
      return mWorkPeriod::ONE;
  }
}
mWorkPeriod mEngineControl::getPreviousWorkPeriod(mWorkPeriod p)
{
  switch (p)
  {
    case mWorkPeriod::ONE :
      return mWorkPeriod::SEVEN;
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
    default :
      return mWorkPeriod::ONE;
  }
} 

void mEngineControl::upPeriodCycle()
{
  if(!this->_isSettingsMode)
    return;

  mEeprom eeprom;
  mDebug debug;
  auto period = mWorkPeriod::ONE;
  eeprom.read(ADDRESS_ENGINE_WORK_MODE, &period);
  
  period = this->getNextWorkPeriod(period);
  this->_currentWorkPeriod = period;
  eeprom.write(ADDRESS_ENGINE_WORK_MODE, period);

  debug.print("Режим изменен на ");
  debug.print((uint8_t)period);
}

void mEngineControl::downPeriodCycle()
{
  if(!this->_isSettingsMode)
    return;

  mEeprom eeprom;
  mDebug debug;
  auto period = mWorkPeriod::ONE;
  eeprom.read(ADDRESS_ENGINE_WORK_MODE, &period);
  
  period = this->getPreviousWorkPeriod(period);
  this->_currentWorkPeriod = period;
  eeprom.write(ADDRESS_ENGINE_WORK_MODE, period);
  debug.print("Режим изменен на ");
  debug.print((uint8_t)period);
}

/**
 * @brief Обновляет индикацию режима работы
*/
void mEngineControl::updateIndications(void)
{
  this->_ledLevel.display(getLedLevel(this->_currentWorkPeriod));
}

/**
 * @brief Конвертирует mWorkPeriod в mLedLevel
*/
LedLevel mEngineControl::getLedLevel(mWorkPeriod workPeriod)
{
  switch (workPeriod)
  {
    case mWorkPeriod::ONE:
      return LedLevel::LEVEL_ONE;
    case mWorkPeriod::TWO:
      return LedLevel::LEVEL_TWO;
    case mWorkPeriod::THREE:
      return LedLevel::LEVEL_THREE;
    case mWorkPeriod::FOUR:
      return LedLevel::LEVEL_FOUR;
    case mWorkPeriod::FIVE:
      return LedLevel::LEVEL_FIVE;
    case mWorkPeriod::SIX:
      return LedLevel::LEVEL_SIX;
    case mWorkPeriod::SEVEN:
      return LedLevel::LEVEL_SEVEN;
    default:
      return LedLevel::LEVEL_ONE;
  }
}

/**
 * @brief Увеличивает частоту ШИМ
*/
void mEngineControl::upPwmFrequency(void)
{
  TCCR2B = 0b00000010;  // x8
  TCCR2A = 0b00000011;  // fast pwm
}

/**
 * @brief Мигает светодиодами
*/
void mEngineControl::blinkIndications(void)
{
  //mDebug debug;
  this->_blinkCounter++;
  if(this->_blinkCounter > this->_maxBlinkCounter)
  {
    this->_blinkCounter = 0;
    if(this->_isIndicationsDisable)
    {
     // debug.print("Светодиоды включились");
      updateIndications();
      this->_isIndicationsDisable = false;
    } else
    { 
     // debug.print("Светодиоды выключились");
      disableIndications();
      this->_isIndicationsDisable = true;
    }
  }
}

/**
 * @brief Включает режим настройки
*/
void mEngineControl::enableSettingsMode(void)
{
  this->_isSettingsMode = true;
  // Настраиваем режим мигания светодиода так, чтобы он сразу начал мигать с выключения
  this->_blinkCounter = this->_maxBlinkCounter;
  this->_isIndicationsDisable = false;
}

/**
 * @brief Выключает режим настройки
*/
void mEngineControl::disableSettingsMode(void)
{
  this->_isSettingsMode = false;
  this->_blinkCounter = 0;
  this->_isIndicationsDisable = false;
}