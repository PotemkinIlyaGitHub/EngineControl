#pragma once
#include <Arduino.h>

// Период работы двигателя в 0,5сек * workPeriod
enum class mWorkPeriod : unsigned char{
    ONE = 1,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    TEN
};

class mEngineControl
{
private:
  enum class engineState
  {
    STOP,
    START,
    RIGHT_ROTATION,
    PREPEAR_LEFT_ROTATION,
    PREPEAR_RIGHT_ROTATION,
    LEFT_ROTATION
  };

  int _leftRotationPin = 0; // Пин управления левым вращением
  int _rightRotationPin = 0;// Пин управления правым вращением
  int _pwmPin = 0;          // Пин шима
  uint8_t _speed = 10;       // Скважность шима, она же скорость вращения
  engineState _state = engineState::STOP;    // Текущее состояние двигателя двигателя
  mWorkPeriod _currentWorkPeriod = mWorkPeriod::ONE; // Текущий период работы двигателя
  int _tick = 0; // Счетчик тиков 10мс для определения режима работы двигателя
  int _timePeriod = 0; // Время работы двигателя в одну сторону
  mWorkPeriod getNextWorkPeriod(mWorkPeriod p);
  mWorkPeriod getPreviousWorkPeriod(mWorkPeriod p); 

public:
  mEngineControl(const int leftRotationPin,
                 const int rightRotationPin,
                 const int pwmPin);

  ~mEngineControl(){};

  void init(void);
  void procedure(void);
  void start(void);
  void stop(void);
  void setLeftRotation(void);
  void setRightRotation(void);
  void upPeriodCycle(void);
  void downPeriodCycle(void);
  inline void setSpeed(const uint8_t speed) { this->_speed = speed; }
  inline void changeTick(void) { this->_tick++; }

};