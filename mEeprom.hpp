#pragma once
#include <Arduino.h>

class mEeprom
{
public:
  mEeprom(){};
  ~mEeprom(){};

  template<typename T>
  void write(const unsigned int startAddress, T data)
  {
    const int size = sizeof(T);
    for(int i = 0; i < size; i++)
    {
      while(eeprom_is_ready() == 0);
      eeprom_write_byte(startAddress+i, *((byte *)&data + i));
      while(eeprom_is_ready() == 0);
    } 
  }
  
  template<typename T>
  void read(const unsigned int startAddress, T *data)
  {
    const int size = sizeof(T);
    for(int i = 0; i < size; i++)
      *((byte *)data + i) = eeprom_read_byte(startAddress+i);
  }
private:
};