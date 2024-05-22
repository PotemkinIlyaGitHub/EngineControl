#include <avr/io.h>
#include <avr/interrupt.h>

#include "mDebug.hpp"
#include "mEeprom.hpp"
#include "mEngineControl.hpp"
#include "mButton.hpp"

#define LEFT_BUTTON (8)
#define RIGHT_BUTTON (7)
#define CONTROL_BUTTON (12)
#define LEDPIN (2)

/***********************************************************
* Для шим двигателя используется TIM2
* Для кнопок используется и обработчика работы двигателя используется TIM1
************************************************************/
void buttonHandler(void);
void initTimer(void);
mDebug debug;

mButton controlButton(CONTROL_BUTTON);   // Кнопка старт/стоп
mEngineControl engineControl(10, 9, 11); // Управление вигателем

// Все настройки и инициализация
void setup()
{
  debug.init(115200); // Инициализация дебага
  // Инициализация кнопок
  controlButton.init();
  // Инициализация управления двигателем
  engineControl.init();
  engineControl.setSpeed(255);
  engineControl.stop();
  initTimer(); // Инициализация таймера для обработки кнопок и работы двигателя
}

// Главный цикл
void loop()
{
  delay(10);
  buttonHandler();
}

// Обработчик событий кнопок
void buttonHandler(void)
{
  if(controlButton.buttonStateIsChange())
  {
    if(engineControl.getEngineState() != mEngineState::STOP)
      return;
      
    if(controlButton.getState() == mButtonState::SHORT_PRESSED)
    {
      // Запуск работы работы двигателя
      debug.print("Control button short pressed");

      if(engineControl.isSettingsMode())
      { 
        engineControl.upPeriodCycle();
        engineControl.updateIndications();
      } else
      {
        if(engineControl.getEngineState() == mEngineState::STOP)
        {
          debug.print("Запуск двигателя");
          engineControl.start();
        }
      }
    } 
    if(controlButton.getState() == mButtonState::LONG_PRESSED)
    {
      // Запуск режима настройки работы двигателя
      debug.print("Control button long pressed");
      if(!engineControl.isSettingsMode())
      {
        debug.print("Вкючение режима настройки");
        engineControl.enableSettingsMode();
      } 
      else
      {
        debug.print("Выкючение режима настройки");
        engineControl.disableSettingsMode();
        engineControl.updateIndications();
      }
    }
  }
}

// Настройка прерывания таймера на 10мс
void initTimer(void)
{
  cli();  // отключить глобальные прерывания
  TCCR1A = 0;   // установить регистры в 0
  TCCR1B = 0;
  OCR1A = 622; // установка регистра совпадения 
  TCCR1B |= (1 << WGM12);  // включить CTC режим 
  TCCR1B |= (1 << CS12); // Установить биты на коэффициент деления 256
  TIMSK1 |= (1 << OCIE1A);  // включить прерывание по совпадению таймера 
  sei(); // включить глобальные прерывания
}

// Обработчик таймера на 10мс
ISR(TIMER1_COMPA_vect)
{
  controlButton.checkState();

  if(engineControl.isSettingsMode())
    engineControl.blinkIndications();
  else
    engineControl.procedure();
}
