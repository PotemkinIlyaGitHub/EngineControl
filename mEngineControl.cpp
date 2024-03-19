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

/**
 * @brief Инициализирует все
 */
void mEngineControl::init(void)
{
    // Увеличим частоту шим
    // upPwmFrequency();
    pinMode (this->_rightRotationPin, OUTPUT);
    pinMode (this->_leftRotationPin, OUTPUT);
    pinMode (this->_pwmPin, OUTPUT);

    this->_currentWorkPeriod = readWorkPeriodFromEeprom();

    mDebug debug;
    debug.print("Инициализация engineControl");
    debug.print("Режим работы: ");
    debug.print((uint8_t) this->_currentWorkPeriod);

 //   mLedPower ledPower;
 //   ledPower.enable();

    updateIndications();
    this->_state = mEngineState::STOP;
}

/**
 * @brief Запускает работу двигателя
 */
void mEngineControl::start()
{
  if(this->_state != mEngineState::STOP)
    return;
  mDebug debug;
  this->_tick = 0;
  this->_timePeriod = 500 * (uint8_t )this->_currentWorkPeriod / 10;
  this->_state = mEngineState::START;
  debug.print("Start");
}

/**
 * @brief Останавливает работу двигателя
 */
void mEngineControl::stopRotation()
{
  analogWrite(this->_pwmPin, 0);
  digitalWrite(this->_leftRotationPin, LOW);
  digitalWrite(this->_rightRotationPin, LOW);
  this->_tick = 0;
}

void mEngineControl::stop()
{
  analogWrite(this->_pwmPin, 0);
  digitalWrite(this->_leftRotationPin, LOW);
  digitalWrite(this->_rightRotationPin, LOW);
  this->_state = mEngineState::STOP; 

  mDebug debug;
  debug.print("Stop");
}

/**
 * @brief Устанавливает левое вращение
 */
void mEngineControl::setLeftRotation()
{
  digitalWrite(this->_leftRotationPin, HIGH);
  digitalWrite(this->_rightRotationPin, LOW);
  analogWrite(this->_pwmPin, this->_speed);
}

/**
 * @brief Устанавливает правое вращение
 */
void mEngineControl::setRightRotation()
{
  digitalWrite(this->_leftRotationPin, LOW);
  digitalWrite(this->_rightRotationPin, HIGH);
  analogWrite(this->_pwmPin, this->_speed);
}

/**
 * @brief Запускает ход основной работы
 */
void mEngineControl::procedure(void)
{
  switch(this->_state)
  {
    case mEngineState::STOP :
    { 
      return;
    }
    case mEngineState::START :
    {
      this->_tick = 0;
      setRightRotation();
      this->_state = mEngineState::RIGHT_ROTATION;
      return;
    }
    case mEngineState::RIGHT_ROTATION :
    {
      this->_tick++;
      if(this->_tick < this->_timePeriod)
        return;

      stopRotation();
      this->_state = mEngineState::PREPEAR_LEFT_ROTATION;
      return;
    }
    case mEngineState::LEFT_ROTATION :
    {
      this->_tick++;
      if(this->_tick < this->_timePeriod)
        return;
      this->_tick = 0;
      stopRotation();
      this->_state = mEngineState::END_CYCLE;
      return;
    }
    case mEngineState::PREPEAR_LEFT_ROTATION :
    {
      this->_tick++;
      if(this->_tick < 20) // Не прошло 200мс
        return;

      this->_tick = 0;
      setLeftRotation();
      this->_state = mEngineState::LEFT_ROTATION;
      return;
    }
    case mEngineState::END_CYCLE:
    {
      stop();
      this->_state = mEngineState::STOP;
      return;
    }
    default:
    {
      stop();
      this->_state = mEngineState::STOP;
    }
  }
}

/**
 * @brief Выбирает следующий режим
 * @param p Текущий режим
 * @return mWorkPeriod Следующий режим работы
 */
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

/**
 * @brief Выбирает предыдущий режим
 * @param p Текущий режим
 * @return mWorkPeriod Предыдущий режим работы
 */
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

/**
 * @brief Увеличивает период работы
 */
void mEngineControl::upPeriodCycle()
{
  if(!this->_isSettingsMode)
    return;

  this->_currentWorkPeriod = this->getNextWorkPeriod(this->_currentWorkPeriod);

  writeWorkPeriodInEeprom(this->_currentWorkPeriod);

  mDebug debug;
  debug.print("Режим изменен на ");
  debug.print((uint8_t)this->_currentWorkPeriod);
}

/**
 * @brief Понижает период работы
 */
void mEngineControl::downPeriodCycle()
{
  if(!this->_isSettingsMode)
    return;

  this->_currentWorkPeriod = this->getPreviousWorkPeriod(this->_currentWorkPeriod);
  writeWorkPeriodInEeprom(this->_currentWorkPeriod);

  mDebug debug;
  debug.print("Режим изменен на ");
  debug.print((uint8_t)this->_currentWorkPeriod);
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

/**
 * @brief Вычитывает режим работы из еепром
 */
mWorkPeriod mEngineControl::readWorkPeriodFromEeprom(void)
{
  mEeprom eeprom;
  mWorkPeriod p;
  eeprom.read(ADDRESS_ENGINE_WORK_MODE, &p);

  return p;
}

/**
 * @brief Вычитывает режим работы из еепром
 */
void mEngineControl::writeWorkPeriodInEeprom(mWorkPeriod &workPeriod)
{
  mEeprom eeprom;
  eeprom.write(ADDRESS_ENGINE_WORK_MODE, workPeriod);
}
