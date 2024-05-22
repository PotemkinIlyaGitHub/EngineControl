#pragma once
#include <Arduino.h>
#include "mLed.hpp"
#include "mDebug.hpp"

// Период работы двигателя в 0,5сек * workPeriod
enum class mWorkPeriod : unsigned char{
    ONE = 1,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN
};

enum class mEngineState
{
  STOP,
  START,
  RIGHT_ROTATION,
  PREPEAR_LEFT_ROTATION,
  PREPEAR_RIGHT_ROTATION,
  LEFT_ROTATION,
  END_CYCLE
};

class mEngineControl
{
private:
  const int _leftRotationPin; // Пин управления левым вращением
  const int _rightRotationPin;// Пин управления правым вращением
  const int _pwmPin;          // Пин шима
  uint8_t _speed = 80;       // Скважность шима, она же скорость вращения. Изменять только методом setSpeed(int)
  mEngineState _state = mEngineState::STOP;    // Текущее состояние двигателя двигателя
  mWorkPeriod _currentWorkPeriod = mWorkPeriod::ONE; // Текущий период работы двигателя
  int _tick = 0; // Счетчик тиков 10мс для определения режима работы двигателя
  int _timePeriod = 0; // Время работы двигателя в одну сторону
  bool _isSettingsMode = false;  // Флаг включен/отключен режим настройки работы двигателя
  bool _isIndicationsDisable = false; // Текущее состояние индикации
  uint8_t _blinkCounter = 0; // Cчетчик для мигания светодиодами
  uint8_t _maxBlinkCounter = 25; // Максимальное количество тиков для блинка светодиодами 25 * 10ms = 250ms
  
  mDebug *_debug = nullptr; 
  
  mWorkPeriod getNextWorkPeriod(mWorkPeriod p);
  mWorkPeriod getPreviousWorkPeriod(mWorkPeriod p); 

  mWorkPeriod readWorkPeriodFromEeprom(void);
  void writeWorkPeriodInEeprom(mWorkPeriod &workPeriod);

  mLedLevel _ledLevel; // Индикация режима работы
  LedLevel getLedLevel(mWorkPeriod workPeriod);

  void upPwmFrequency(void);
  void setLeftRotation(void);
  void setRightRotation(void);
  void changeStateIndications(void);

  inline void disableIndications(void) { this->_ledLevel.disableAll(); }
  inline void changeTick(void) { this->_tick++; }

public:
  mEngineControl(const int leftRotationPin,
                 const int rightRotationPin,
                 const int pwmPin);

  ~mEngineControl(){delete this->_debug; };

  void init(void);
  void procedure(void);
  void start(void);
  void stop(void);
  void stopRotation(void);
  void upPeriodCycle(void);
  void downPeriodCycle(void);
  void updateIndications(void);
  void blinkIndications(void);
  void enableSettingsMode(void);
  void disableSettingsMode(void);

  inline mEngineState getEngineState(void) { return this->_state; }
  inline void setSpeed(const uint8_t speed) { this->_speed = speed; }
  inline bool isSettingsMode(void) { return this->_isSettingsMode; }

};