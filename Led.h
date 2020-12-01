#ifndef MY_LED_H
#define MY_LED_H

#include <Arduino.h>

class Led {
  
  private:
    byte pin;
    byte _on = LOW;
    byte _off = HIGH;
    
  public:
    Led();
    virtual ~Led();
    void begin(byte pin);
    void setON();
    void setOFF();
    void setInvertido(bool invertido);
};

#endif
