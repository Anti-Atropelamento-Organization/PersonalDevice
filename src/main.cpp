#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

#include "LoRaBoards.h"
#include "DeviceBase.h"
#include "vehicleDevice.h"
#include "PersonalDevice.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include "BLEHandle.h"

#include "mainFunctions.h"
#include "esp_system.h"   
#include "SimpleTimer.h"

PersonalDevice personal;
mainFunctions MF;


SimpleTimer st_safety(30000);
SimpleTimer st_monitoring(3000);
SimpleTimer alertTimer(3000);
SimpleTimer timerbloqueante(5000); 


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
  MF.SendTime(personal, st_monitoring, hasTarget, level, lastLevel);

  if(st_safety.isReady()){
    MF.SendPacketSafety(personal, st_safety, jitterTargetTimeSafety);
  }

  if(st_monitoring.isReady()){
    if((!ackMonitoring || !ackLog) && timerAck == false){
      MF.SendPacketLog(personal, st_monitoring, jitterTargetTimeMonitoring, ackMonitoring, ackLog);
      timerAck = true;
    }else if(timerbloqueante.alreadyGoal(2000) && timerAck){
      MF.SendPacketLog(personal, st_monitoring, jitterTargetTimeMonitoring, ackMonitoring, ackLog);
    }else if(timerbloqueante.isReady() && timerAck){
      timerAck = false;
      st_monitoring.reset();
    }
  }
  



  if (alertTimer.isReady()) {
    MF.ActiveAlert(personal);
    alertTimer.reset();
  }
  personal.cleanOldVehicles();
}

