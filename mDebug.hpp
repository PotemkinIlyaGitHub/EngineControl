#pragma once
#include <Arduino.h>

class mDebug
{
public:
  mDebug(){};
  ~mDebug(){};
  void init(unsigned long baudRate);
  void print(const unsigned long numeric, const int base = DEC);
  void print(const char *str);
};
