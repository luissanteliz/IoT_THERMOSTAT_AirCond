#include "Button.h"

Button::Button() {

}

Button::~Button() {

}

void Button::begin(byte pin, int pinmode) {
  this->pin = pin;
 
  lastReading = _off;
  
  pinMode(pin, pinmode);
  update();
}

void Button::setInvertido(bool invertido) {
  if(invertido) {
    this->_off = HIGH;
    this->_on = LOW;
  } else {
    this->_off = LOW;
    this->_on = HIGH;
  }
}

void Button::update() {
    // You can handle the debounce of the button directly
    // in the class, so you don't have to think about it
    // elsewhere in your code
    byte newReading = digitalRead(pin);
    
    if (newReading != lastReading) {
      lastDebounceTime = millis();
    }

    if (millis() - lastDebounceTime > debounceDelay) {
      // Update the 'state' attribute only if debounce is checked
      state = newReading;
    }

    lastReading = newReading;
}

byte Button::getState() {
  update();
  return state;
}

bool Button::isPressed() {
  return (getState() == this->_on);
}
