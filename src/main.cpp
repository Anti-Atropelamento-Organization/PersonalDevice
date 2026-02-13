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

unsigned long jitterTargetTime = 0;
bool waitingToSend = false;
bool hasTarget = false;
int level = 3;
static int lastLevel = -1;

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
  MF.ReceivePacketDevice(personal, st_safety, jitterTargetTime, waitingToSend, hasTarget);
  MF.SendTime(personal, st_monitoring, hasTarget, level, lastLevel);
  MF.SendPacketDevice(personal, st_safety, st_monitoring, jitterTargetTime);
  if (alertTimer.isReady()) {
    MF.ActiveAlert(personal);
    alertTimer.reset();
  }
  personal.cleanOldVehicles();
}

