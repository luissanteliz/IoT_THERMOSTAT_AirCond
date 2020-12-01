/**
 * Termostato programable para aire acondicionado
 * Autor: luis santeliz <luissanteliz@gmail.com>
 * versión: 0.1
 * fecha: 11/11/2020
 */

#include "notas.h"
#include "relayctrl.h"
#include "Button.h"
#include "Led.h"
#include <driver/adc.h>

#define _DEBUG

#define TONE_DURATION_DIVIDER 500
 #define ADC_RES 4095.0
// #define ADC_RES 1023.0


RELAYCTRL compresor;
RELAYCTRL ventiladorM; // velocidad media
RELAYCTRL ventiladorH; // velocidad alta
RELAYCTRL abanico; // motor abanico




Button btnUp;
Button btnDn;
Button btnFan;
Button btnPwr;

Led ledRojo;
Led ledVerde;
Led ledBuiltIn;

#define _TEMP_IDEAL_INI 18

const int TEMP_MIN = 15; // temperatura minima
const int TEMP_MAX = 40; // temperatura maxima
const long RELE1_SEC_OFF_MINDELAY = 60*1000; // (milliseg) indica el tiempo que debe estar
                                        // el compresor apagado/OFF antes de volver a encender
                                        // evita encender y apagar el motor muy rapido

const long RELE1_SEC_ON_MAXDELAY =  960*1000; // (milliseg) indica el tiempo que puede estar
                                          // el compresor encendido/ON
                                          // evita sobrecalentamiento entre otros

#define INIT_SEC_OFF_TIME 30000 // tiempo apagado al encender el sistema

float tempActual = 0;
float tempIdeal = 18; // siempre comienza en 18
byte fanPos = 3;
boolean releON = false;
int tecla = 0;
int horaOFF = 0;
int horaON = 0;
int horaActual =0;
int error = 0; // si >0 entonces hay un error
// 20: tiempo de espera minimo RELE1_SEC_OFF_MINDELAY aun no alcanzado
// 30: tiempo de encendido RELE1_SEC_ON_MAXDELAY del motor excedido

int count = 0;

int Vo;
float R1 = 100000;              // resistencia fija del divisor de tension 
float logR2, R2;
float c1 = -1.530374480e-03, c2 = 5.771239158e-04, c3 = -9.722278060e-07;
// coeficientes de S-H en pagina: 
// http://www.thinksrs.com/downloads/programs/Therm%20Calc/NTCCalibrator/NTCcalculator.htm

const int BUZZ = 4;

// INPUT's
#define BTN_UP 14
#define BTN_DOWN 12
#define BTN_FAN 13
#define BTN_PWR 15

// OUTPUT's
#define RLY_CMP 21
#define RLY_FLW 25

#define LED_ROJ 26
#define LED_VER 27

#define RLY_FANH 33
#define RLY_FANM 32

const int NTC1_SENSOR = 34;

void IRAM_ATTR pwrISR() {
  if(btnPwr.isPressed()) {
    init();
    error = 0;
    ledBuiltIn.setON();
  } else {
    stopall();
    error = 99;
    ledBuiltIn.setOFF();
    esp_deep_sleep_start();
  }
};


void setup() {
  btnUp.setInvertido(true);
  btnUp.begin(BTN_UP, INPUT_PULLUP); // Boton arriba
  
  btnDn.setInvertido(true);
  btnDn.begin(BTN_DOWN, INPUT_PULLUP); // Boton Abajo
  
  btnFan.setInvertido(true);
  btnFan.begin(BTN_FAN, INPUT_PULLUP); // Boton FAN
  
  btnPwr.setInvertido(false);
  btnPwr.begin(BTN_PWR,INPUT_PULLUP); // Boton Power ENCIENDE CON HIGH
  attachInterrupt(BTN_PWR, pwrISR, CHANGE);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_15,1); //1 = High, 0 = Low
  
  pinMode(BUZZ, OUTPUT); // salida buzzer
  
  adc1_config_width(ADC_WIDTH_BIT_10);
  adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_DB_0);
  Serial.begin(9600);   // inicializa comunicacion serie a 9600   bps

  // inicializacion de reles
  compresor.begin(RLY_CMP); 
  compresor.setLedPinOut(LED_ROJ); // led indicador
  ventiladorM.begin(RLY_FANM);
  ventiladorM.setLedPinOut(LED_VER); // led indicador
  ventiladorH.begin(RLY_FANH);
  ventiladorM.setLedPinOut(LED_VER); // led indicador
  abanico.begin(RLY_FLW);

  ledRojo.setInvertido(true);
  ledRojo.begin(LED_ROJ);
  ledVerde.setInvertido(true);
  ledVerde.begin(LED_VER);
  
  ledBuiltIn.begin(LED_BUILTIN);

  // otros inits;
  tempIdeal = _TEMP_IDEAL_INI;
  horaActual = millis();
  horaOFF = 0;

  // verificar Power Button, si estaba apagado al inicial la energia 
  // despues de un apagon por ejemplo
  pwrISR(); 
  fanSetSpeed(fanPos);
}



void loop() {
//  int btnUP = digitalRead(BTN_UP);
//  int btnDOWN = digitalRead(BTN_DOWN);
//  int btnFAN = digitalRead(BTN_FAN);

  if(error==99) {
    
  #ifdef _DEBUG
    Serial.println("off");
    if(millis()-horaOFF>5000) {
      Serial.println("off");
    }
  #endif
    
    horaOFF = millis();
    return;
  } // error 99
  int tecla;
  tecla = digitalRead(BTN_PWR);
  Serial.print("BTNPWR ");
  Serial.print(btnUp.isPressed());
  Serial.println(tecla);
  
  if(btnUp.isPressed()) { // && tempIdeal<TEMP_MAX
    tempIdeal=tempIdeal+1;
    Serial.println("Ideal|++");
    btnUpTone();
  }

  if(btnDn.isPressed() ) { // && tempIdeal>TEMP_MIN
    tempIdeal=tempIdeal-1;
    Serial.println("Ideal--");
    btnDownTone();
  }

  if(btnFan.isPressed()){
    
    if(fanPos==2) {
      fanPos=3;
    } else {
      --fanPos;
    }
    Serial.print("SET FAN ");
    Serial.println(fanPos);
    fanSetSpeed(fanPos);
  }


  Vo = analogRead(NTC1_SENSOR);      // lectura de NTC1_SENSOR
  #ifdef _DEBUG
    Serial.print("NTC1_SENSOR:");
    Serial.println(Vo);
  #endif
  
  R2 = R1 * (ADC_RES / (float)Vo - 1.0); // conversion de tension a resistencia
  logR2 = log(R2);      // logaritmo de R2 necesario para ecuacion
  tempActual = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2)); // ecuacion S-H
  tempActual = tempActual - 273.15 + 20;   // Kelvin a Centigrados (Celsius)

  horaActual = millis();
  #ifdef _DEBUG
    Serial.print("horaActual ");
    Serial.println(horaActual);  
  #endif
  if(releON) {
      
    #ifdef _DEBUG
      Serial.print("horaON ");
      Serial.print(horaON);
      Serial.print(" : tiempo ON ");
      Serial.print(horaActual-horaON);
      Serial.print(" > ");
      Serial.println(RELE1_SEC_ON_MAXDELAY);
    #endif
      
    if((horaActual-horaON)> RELE1_SEC_ON_MAXDELAY) {
      error = 30; // excedió el tiempo de encendido del motor
      motorOFF();
      Serial.println("ERR30: excedió el tiempo de encendido del motor");
//    } else {
//      error =0;
    }
  } else { // entonces releOFF *----------------
    #ifdef _DEBUG
      Serial.print("horaOFF ");
      Serial.print(horaOFF);
      Serial.print(" : tiempo OFF ");
      Serial.print(horaActual-horaOFF);
      Serial.print(" < ");
      Serial.println(RELE1_SEC_OFF_MINDELAY);
    #endif
    
    if( (horaActual-horaOFF)< RELE1_SEC_OFF_MINDELAY )  { 
      error = 20; // 20: tiempo de espera minimo RELE1_SEC_OFF_MINDELAY aun no alcanzado

      motorOFF();
      Serial.println("ERR20: tiempo de espera minimo aun no alcanzado");
    } else if(error=20) {
      error =0;
    }
  }
  
  if((tempActual-2)>tempIdeal && !releON && error<20) {
    motorON();
  }
  
  if(tempActual<=tempIdeal && releON) {
    motorOFF();
  }

  Serial.print("Act: ");  // imprime valor en monitor serie
  Serial.print(tempActual);
  Serial.print(" °C ");
  Serial.print("Ideal: ");  // imprime valor en monitor serie
  Serial.print(tempIdeal);
  Serial.print(" °C ");
  
  if(releON) {
    Serial.print(" rele ON ");  // imprime valor en monitor serie
  }
  if(error>0) {
    Serial.print(" err:");
    Serial.print(error);
  }
  Serial.println(".");
  
  delay(1000);       // demora de medio segundo entre lecturas
}

void motorON() {
    if(error==99) { //  el equipo esta apagado
      return;
    }
    if(error>=30) { // si hay warning o error crítico entonces no enciende el motor.
      Serial.print("ERROR CRITICO:");
      Serial.println(error);
      Serial.print(" imposible encender");
      return;
    }
//    digitalWrite(RELE1,HIGH); // encender RELE1 *(APAGA CON LOW)
//    digitalWrite(LED_BUILTIN,HIGH); // encender LED_BUILTIN
    compresor.setON();
    if(!releON) {
      releON = true;
      horaON = horaActual;
      horaOFF = 0;
    }
};


void motorOFF() {
//    digitalWrite(RELE1,LOW); // apagar RELE1 *(APAGA CON LOW)
//    digitalWrite(LED_BUILTIN,LOW); // apagar LED_BUILTIN
    compresor.setOFF();
    if(releON){
      releON = false;
//    if(error<=30) { // si hay error critico no borrar horaON
      horaON = 0;
//    }
      horaOFF = horaActual;  
    }
};

void fanSetSpeed(byte fanSpd) {
      switch(fanSpd){
      case 2:
        ventiladorH.setOFF(); 
        ventiladorM.setON();        
        break;
      case 3:
        ventiladorH.setON();
        ventiladorM.setOFF(); 
        break;
      default:
    }
}


void btnUpTone() {
//    int melody[] = {NOTE_D3, NOTE_C3};
//    int noteDurations[] = {4, 4};
    tone(BUZZ, NOTE_C7, TONE_DURATION_DIVIDER/4);

  }

void btnDownTone() {
//    int melody[] = { NOTE_C3, NOTE_D3 };
//    int noteDurations[] = {4, 4};
      tone(BUZZ, NOTE_C5, TONE_DURATION_DIVIDER/4);
  }

void melodyON() {
    // notes in the melody:
    int melody[] = {NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4};
    
    // note durations: 4 = quarter note, 8 = eighth note, etc.:
    int noteDurations[] = {4, 8, 8, 4, 4, 4, 4, 4};
    
    partitureplay(melody,noteDurations);
}


void partitureplay(int partiture[], int durations[]) {
      for (int thisNote = 0; thisNote < 8; thisNote++) {
        int noteDuration = TONE_DURATION_DIVIDER / durations[thisNote];
        tone(BUZZ, partiture[thisNote], noteDuration);
        int pauseBetweenNotes = noteDuration * 1.10;
        delay(pauseBetweenNotes);
        // stop the tone playing:
        noTone(BUZZ);
      }
}


void tone(byte pin, int freq,int duration) {
  ledcSetup(0, 2000, 8); // setup beeper
  ledcAttachPin(pin, 0); // attach beeper
  
  ledcWriteTone(0, freq); // play tone
  delay(duration);
  ledcWriteTone(0, 0); // play tone
}
void noTone(int pin) {
  tone(pin, 0,1);
}

void IRAM_ATTR init() {
  melodyON();
  motorOFF();                // compresor
  ventiladorM.setOFF();
  ventiladorH.setON();
  abanico.setOFF();
//  ledBuiltIn.setON();
};

void IRAM_ATTR stopall() {
    compresor.setOFF();
    ventiladorM.setOFF();
    ventiladorH.setOFF();
    abanico.setOFF();
    ledRojo.setOFF();
    ledVerde.setOFF();
};
