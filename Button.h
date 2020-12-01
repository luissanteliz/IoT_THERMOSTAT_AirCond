#ifndef MY_BUTTON_H
#define MY_BUTTON_H
#include <Arduino.h>
class Button {
  
  private:
    byte pin;
    byte state;
    byte lastReading;
    unsigned long lastDebounceTime = 0;
    unsigned long debounceDelay = 1000;
    byte _on = HIGH;
    byte _off = LOW;
    
  public:
    Button();
    virtual ~Button();
    void begin(byte pin, int pinmode);
    void update();
    byte getState();
    bool isPressed();
    void setInvertido(bool invertido);
};
#endif
