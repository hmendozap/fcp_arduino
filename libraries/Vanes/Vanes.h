#ifndef Vanes_h
#define Vanes_h

#include "Arduino.h"

class Vanes{
  public:
    Vanes(int pin);
    float getVal();
    void calibrate();

  private:
    int _pin;
    int _i;
    volatile int _signalLength;
    volatile unsigned long _high;
    volatile unsigned long _low;
    float	_angle;
};
#endif
