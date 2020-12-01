/**
 * Control de RELE

 * Autor: luis santeliz <luissanteliz@gmail.com>
 * versión: 0.1
 * fecha: 21/11/2020
 */

#ifndef RELAYCTRL_H
#define RELAYCTRL_H

#include <Arduino.h>

class RELAYCTRL  {
	public:
		RELAYCTRL(); // constructor
		virtual ~RELAYCTRL();
		void begin(int opin);
		int getOutPin(); // pin de salida
		bool isON();  // obtiene estado 0:apagado, 1:encendido
		void setON(); // encender
		void setOFF(); // apagar
		void setInvertido(bool invertido);
		unsigned long getReleSecOffMindelay();
		void setReleSecOffMindelay(int millisec);
		unsigned long getReleSecOnMaxdelay();
		void setReleSecOnMaxdelay(int millisec);
		void setLedPinOut(int ledPinO); // pin led indicador, al inicio es 0
		int getLedPinOut();


	private:
		int _opin = -1; // pin de salida
		byte _state = -1; // 0:apagado, 1:encendido
		bool _invertido = false; //    si es true el rele apaga con LOW 
		byte _apagado = LOW;
		byte _encendido = HIGH;
		int _ledPinOut = 0;

		// (milliseg) indica el tiempo que debe estar
		// el compresor apagado/OFF antes de volver a encender
	    // evita encender y apagar el rele muy rapido
		unsigned long _rele_sec_off_mindelay = 90000;

 		// (milliseg) indica el tiempo que puede estar
 		// el rele encendido/ON
 		// evita sobrecalentamiento entre otros
		unsigned long _rele_sec_on_maxdelay =  240000;

		// tiempo minimo apagado en el primer encendido
		unsigned long _init_sec_off_time = 30000;

};


#endif // RELAYCTRL_H
