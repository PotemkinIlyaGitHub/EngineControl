#include "mDebug.hpp"
#include <HardwareSerial.h>

void mDebug::init(unsigned long baudRate)
{
  Serial.begin(baudRate);
}

void mDebug::print(const unsigned long numeric, const int base)
{
  Serial.print(numeric, base);
  Serial.print("\r\n");
}

void mDebug::print(const char *str)
{
  Serial.print(str);
  Serial.print("\r\n");
}