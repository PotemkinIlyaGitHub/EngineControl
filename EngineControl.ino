#include <avr/io.h>
#include <avr/interrupt.h>

#include "mDebug.hpp"
#include "mEeprom.hpp"
#include "mEngineControl.hpp"
#include "mButton.hpp"

#define LEFT_BUTTON (8)
#define RIGHT_BUTTON (7)
#define CONTROL_BUTTON (6)
#define LEDPIN (2)

/***********************************************************
* Для шим двигателя используется TIM2
* Для кнопок используется TIM1
* Для обработчика работы двигателя используется TIM3
************************************************************/
void buttonHandler(void);
void initTimer(void);
mDebug debug;

mButton leftButton(LEFT_BUTTON);         // Кнопка уменьшения периода работы двигателя/уменьшения скорости вращения
mButton rightButton(RIGHT_BUTTON);       // Кнопка увеличения периода работы двигателя/увеличения скорости вращения
mButton controlButton(CONTROL_BUTTON);   // Кнопка старт/стоп
mEngineControl engineControl(10, 9, 11); // Управление вигателем

// Все настройки и инициализация
void setup()
{
  debug.init(115200); // Инициализация дебага
  initTimer(); // Инициализация таймера для обработки кнопок и работы двигателя
 
  // Инициализация кнопок
  leftButton.init();
  rightButton.init();
  controlButton.init();

  // Инициализация управления двигателем
  engineControl.init();
  engineControl.setSpeed(250);
}

// Главный цикл
void loop()
{
  delay(1); // Задержка, так как длительное нажатие может успеть несколько раз в миллисекунду обработаться
  buttonHandler();
}

void buttonHandler(void)
{
  if(leftButton.buttonStateIsChange())
  {
    if(leftButton.getState() == mButtonState::SHORT_PRESSED)
    {
      // Уменьшить период работы двигателя
      debug.print("Left button short pressed");
      engineControl.downPeriodCycle();
    }
 
    if(leftButton.getState() == mButtonState::LONG_PRESSED)
    {
      // Уменьшить скорость вращения двигателем
      debug.print("Left button long pressed");
    }
  }
  
  if(rightButton.buttonStateIsChange())
  {
    if(rightButton.getState() == mButtonState::SHORT_PRESSED)
    {
      // Увеличить период работы двигателя
      debug.print("Right button short pressed");
      engineControl.upPeriodCycle();
    }

    if(rightButton.getState() == mButtonState::LONG_PRESSED)
    {
      // Увеличить скорость вращения двигателем
      debug.print("Right button long pressed");
    }
  }

  if(controlButton.buttonStateIsChange())
    if(controlButton.getState() != mButtonState::NO_PRESSED)
    {
      engineControl.start();
      // Изменить состояние работы двигателя
      debug.print("Control button pressed");
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
  leftButton.checkState();
  rightButton.checkState();
  controlButton.checkState();
  engineControl.procedure();
}
