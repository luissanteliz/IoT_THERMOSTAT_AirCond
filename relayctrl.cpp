/**
 * Control de RELE

 * Autor: luis santeliz <luissanteliz@gmail.com>
 * versión: 0.1
 * fecha: 21/11/2020
 */
#include "relayctrl.h"
#include <Arduino.h>


RELAYCTRL::RELAYCTRL() { // constructor 
}

RELAYCTRL::~RELAYCTRL() { 
}

void RELAYCTRL::begin(int opin) { 
  this->_opin = opin;
  pinMode(_opin, OUTPUT);
  setOFF();
}


int RELAYCTRL::getOutPin() { // pin de salida 
	return _opin;
}

bool RELAYCTRL::isON() {  // obtiene estado 0:apagado, 1:encendido 
	if(_state==1) {
		return true;
	} else {
		return false;
	}
}

void RELAYCTRL::setON() { // encender 
  	digitalWrite(_opin,_encendido); // encender RELE
    if(_ledPinOut>0) {
    	digitalWrite(_ledPinOut,LOW); //  encender LED indicador
    }
    _state = 1;
}

void RELAYCTRL::setOFF() { // apagar 
    digitalWrite(_opin,_apagado); // apagar RELE
    if(_ledPinOut>0) {
    	digitalWrite(_ledPinOut,HIGH); //  apagar LED indicador
    }
    
    _state = 0;
    
}


//   si es true la salida del rele es LOW para encendido Y HIGH para apagado
void RELAYCTRL::setInvertido(bool invertido) {
	_invertido = invertido;
	if(_invertido) {
		_apagado = HIGH;
		_encendido = LOW;
	} else {
		_apagado = LOW;
		_encendido = HIGH;
	}

}

unsigned long RELAYCTRL::getReleSecOffMindelay() { 
	return _rele_sec_off_mindelay;
}

void RELAYCTRL::setReleSecOffMindelay(int millisec) { 
	_rele_sec_off_mindelay = millisec;
}

unsigned long RELAYCTRL::getReleSecOnMaxdelay() { 
	return _rele_sec_on_maxdelay;
}

void RELAYCTRL::setReleSecOnMaxdelay(int millisec) { 
	_rele_sec_on_maxdelay = millisec;

}

void RELAYCTRL::setLedPinOut(int ledPinO) {
	_ledPinOut = ledPinO;
}
int RELAYCTRL::getLedPinOut() {
	return _ledPinOut;
}
