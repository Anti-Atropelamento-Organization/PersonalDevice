#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

#include "LoRaBoards.h"
#include "DeviceBase.h"
#include "SimpleTimer.h"
#include "PersonalDevice.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include "BLEHandle.h"

#define MOVING_SAFETY_TIMER 1000
#define STOPPED_SAFETY_TIMER 5000
#define MONITORING_TIMER 60000

SimpleTimer safetyTimer(STOPPED_SAFETY_TIMER);        
SimpleTimer monitoringTimer(MONITORING_TIMER); 
SimpleTimer simTimer(50000);   

unsigned long lastMillis = 0;

/*
Aqui foi implementado todos os métodos da classe device, mas como a classe device é abstrata, vai ser utilizado a classe PersonalDevice que herda da DeviceBase
*/

SimpleTimer safetyTimer(1000);

PersonalDevice personal;

// Cria o manipulador BLE com referência ao dispositivo pessoal
BLEHandle ble(personal); 


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

  ble.begin("CRACHA_LORA_01"); 
  Serial.println("Aguardando configuração...");
  while (personal.getID() == 0) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("\nConfigurado! Iniciando loop...");
  
  randomSeed(digitalRead(0));
  lastMillis = millis();
}


void loop() {

/*
##########################
FUNCIONALIDADES BÁSICAS
##########################
*/
  personal.alimentandoGPS();
  
  bool fixOk   = hasFixSimple();
  bool satValid = personal.getSatValid();
  uint32_t sats = satValid ? personal.getSatValue() : 0;
  
  if (fixOk) {
    personal.setLatitude();
    personal.setLongitude();
    personal.setSpeed();
    personal.setCourse();
    personal.setHdop();
    personal.setRadius(personal.getHdop());
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
ENVIO DE PACOTE DESDE DISPOSITIVO
##########################
*/
  if (safetyTimer.isReady()) {
    if (personal.hasLocation() && !personal.isChannelBusy(SAFETY_CHANNEL)) personal.sendSafety();
    safetyTimer.reset();
  }


/*
##########################
PACOTES DE MONITORAMENTO
##########################
*/

  if (monitoringTimer.isReady()) {
    Serial.println("Enviando pacote de monitoramento..."); 
    if (personal.isChannelBusy(MONITORING_CHANNEL)) {
            Serial.println("Canal ocupado");

            monitoringTimer.setInterval(random(500, 1000));
            monitoringTimer.reset();
            
            // Backoff: Tenta novamente em breve
            monitoringTimer.setInterval(random(500, 1000));
            monitoringTimer.reset();
    } else {
            Serial.println("Sending Monitoring Packet...");
            personal.sendMonitoring();
            personal.setSpeed();
            
            monitoringTimer.setInterval(MONITORING_TIMER);
            monitoringTimer.reset();
    }
    monitoringTimer.reset();
  }

}

