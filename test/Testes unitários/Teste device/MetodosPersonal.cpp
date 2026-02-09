#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

#include "LoRaBoards.h"
#include "DeviceBase.h"
#include "SimpleTimer.h"
#include "PersonalDevice.h"

/*
Aqui foi implementado todos os métodos da classe device, mas como a classe device é abstrata, vai ser utilizado a classe PersonalDevice que herda da DeviceBase
*/

SimpleTimer safetyTimer(1000);

PersonalDevice personal;


// função que verica se fix ja é true, ou seja, se o GPS ja tem um valor válido
static inline bool hasFixSimple() {
  return personal.hasLocation() && personal.getSatValid() && personal.getSatValue() >= 4;
}

void setup() {
  Serial.begin(115200);
  delay(200);

  setupBoards();
  delay(300);

  personal.setID(1);
  personal.setup();
}


void loop() {

/*
##########################
MÉTODO DE ENVIO DE ACORDO COM A DISNTâNCIA DO VEÍCULO MAIS PRÓXIMO
##########################
*/
  personal.alimentandoGPS();
  
  bool fixOk = hasFixSimple();
  
  if (fixOk) {
    personal.setLatitude();
    personal.setLongitude();
  }

 /*
##########################
PACOTE RECEBIDO DE OUTRO DISPOSITIVO
##########################
*/
  static float targetLat = 0.0f;
  static float targetLng = 0.0f;
  static bool  hasTarget = false;

  if (personal.receive()) {
    targetLat = personal.getReceivedLat();
    targetLng = personal.getReceivedLng();
    hasTarget = true;
  }

/*
##########################
MÉTODO DE NÍVEL DE ENVIO
##########################
*/
  int level = 3;
  if (hasTarget) {
    level = personal.isValidSend(targetLat, targetLng);
  }

  static int lastLevel = -1;
  if (level != lastLevel) {
    if (level == 1) safetyTimer.setInterval(1000);
    else if (level == 2) safetyTimer.setInterval(5000);
    safetyTimer.reset();
    lastLevel = level;
  }
}

