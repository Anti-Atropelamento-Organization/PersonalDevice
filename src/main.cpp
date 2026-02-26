#include <Arduino.h>
// #include <Wire.h>
// #include <U8g2lib.h>

#include "LoRaBoards.h"
#include "DeviceBase.h"
#include "PersonalDevice.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include "BLEHandle.h"

#include "mainFunctions.h"
#include "esp_system.h"   
#include "SimpleTimer.h"

PersonalDevice personal;
mainFunctions MF;


SimpleTimer st_safety(5000);
SimpleTimer st_monitoring(180000);
SimpleTimer alertTimer(3000);
SimpleTimer timerbloqueante(10000); 
SimpleTimer teste(1000);

uint8_t cont = 1;

bool timerAck = false;

unsigned long jitterTargetTimeSafety = 0;
unsigned long jitterTargetTimeMonitoring = 0;
bool waitingToSend = false;
bool hasTarget = false;
int level = 3;
static int lastLevel = -1;

bool ackMonitoring = false;
bool ackLog = false;

uint16_t randomMonitoring = 0;
uint16_t randomLog = 0;

void setup() {

  Serial.begin(115200);
  delay(200);

  setupBoards();
  delay(300);

  personal.setID(12);
  personal.setup();

  randomSeed((uint32_t)esp_random() ^ (uint32_t)micros());
}

void loop() {

  MF.SetPersonalConst(personal);
  MF.ReceivePacketDevice(personal, st_safety, jitterTargetTimeSafety, waitingToSend, hasTarget, ackMonitoring, ackLog);
  MF.SendTime(personal, st_safety, hasTarget, level, lastLevel);

  if(st_safety.isReady()){
    MF.SendPacketSafety(personal, st_safety, jitterTargetTimeSafety);
  }

  if(st_monitoring.isReady()){
    
    if(ackMonitoring && cont <= 3){
      cont = 4;
    }
    
    if(ackLog){
      st_monitoring.reset();
      timerAck = false;
      ackMonitoring = false;
      ackLog = false;
      cont = 1;

    }

    else if(cont == 1 && !timerAck && !ackMonitoring){
      MF.SendPacketLog(personal, st_monitoring, jitterTargetTimeMonitoring, false, true); 
      timerAck = true;
      timerbloqueante.reset();
      Serial.println("Enviando MONITORING (Tentativa 1) - cont: " + String(cont));
      cont = 2;
    } 
    
    else if(cont == 2 && !ackMonitoring && timerbloqueante.alreadyGoal(2000)){
      MF.SendPacketLog(personal, st_monitoring, jitterTargetTimeMonitoring, false, true);
      Serial.println("Enviando MONITORING (Tentativa 2) - cont: 2");
      cont = 3;
    }
   
    else if(cont == 3 && (ackMonitoring || timerbloqueante.alreadyGoal(4000))){
      if(!ackMonitoring) Serial.println("Timeout Monitoring...");
      timerbloqueante.reset();
      cont = 4; 
    }
   
    else if(cont == 4 && !ackLog){
      MF.SendPacketLog(personal, st_monitoring, jitterTargetTimeMonitoring, true, false); // Mudei para (true, false) para enviar LOG
      Serial.println("Enviando LOG (Tentativa 1) - cont: 4");
      timerbloqueante.reset(); 
      cont = 5;
    }
   
    else if(cont == 5 && !ackLog && timerbloqueante.alreadyGoal(2000)){
      MF.SendPacketLog(personal, st_monitoring, jitterTargetTimeMonitoring, true, false);
      Serial.println("Enviando LOG (Tentativa 2) - cont: 5");
      cont = 6;
    }
  
    else if(cont == 6 && (ackLog || timerbloqueante.alreadyGoal(4000))){
      if(!ackLog) Serial.println("Timeout Log...");
      
      Serial.println("Ciclo completo. Resetando timers.");
      st_monitoring.reset();
      timerAck = false;
      ackMonitoring = false;
      ackLog = false;
      cont = 1;
    
    }
}

  if (alertTimer.isReady()) {
    MF.ActiveAlert(personal);
    alertTimer.reset();
  }

  MF.MonitoringEvent(personal);

  personal.cleanOldVehicles();

  if(teste.isReady()){
    Serial.println("INFOS RAIOS E DISTANCIA MINIMA");
    Serial.println("minDistance: " + String(personal.minDistanceFromVehicle()));
    Serial.println("Raio 1: " + String(personal.getRadius(1) - personal.getRadius(0)));
    Serial.println("Raio 2: " + String(personal.getRadius(2) - personal.getRadius(1)));
    Serial.println();
    teste.reset();
  }
  

  /*if(teste.isReady()){
    Serial.println("Sats: " + String(personal.getSatValue()));
    teste.reset();
  } */
}
