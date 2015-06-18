#include "Arduino.h"
#include <Vanes.h>

Vanes::Vanes(int pin) {
  pinMode(pin, INPUT);
  _pin = pin;
}

//Vanes::~Vanes(){/*nothing to destruct*/}

float Vanes::getVal() {
  _high = pulseIn(_pin, HIGH);
  _low = pulseIn(_pin, LOW);

  _angle = (((float) _high * _signalLength)/((float) _high + (float) _low)) - 1;

  _angle = _angle * 359.91 / _signalLength;

  return _angle;
}

void Vanes::calibrate() {
  _high = 0;
  _low = 0;

  for(_i = 0; _i < 500; _i++) {
    if(_i < 2 ) {
		Serial.print("high: ");
		Serial.println(_high);
		Serial.print("low: ");
		Serial.println(_low);
    }
    _high += pulseIn(_pin, HIGH);
    _low += pulseIn(_pin, LOW);
  }

  _signalLength = (_high + _low) / _i;
	Serial.print("signal Length: ");
	Serial.println(_signalLength);
}
