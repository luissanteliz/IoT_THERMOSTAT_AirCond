#include "Led.h"

Led::Led() {
}

Led::~Led() {
}

void Led::begin(byte pin) {
  this->pin = pin;
  pinMode(pin, OUTPUT);
  setOFF();
}

void Led::setON() {
  digitalWrite(pin, _on);
}

void Led::setOFF() {
  digitalWrite(pin, _off);
}

void Led::setInvertido(bool invertido) {
  if(invertido) {
    _off = HIGH;
    _on = LOW;
  } else {
    _off = LOW;
    _on = HIGH;
  }

}
