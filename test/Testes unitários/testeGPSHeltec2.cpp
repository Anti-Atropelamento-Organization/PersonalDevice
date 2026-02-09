#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

#include "LoRaBoards.h"
#include "DeviceBase.h"
#include "PersonalDevice.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include "BLEHandle.h"

#include "mainFunctions.h"
#include "esp_system.h"
#include "SimpleTimer.h"

#define VGNSS_CTRL 3

mainFunctions MF;
PersonalDevice personal;

SimpleTimer st(3000);

unsigned long jitterTargetTime = 0;
bool waitingToSend = false;

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(VGNSS_CTRL,OUTPUT);
  digitalWrite(VGNSS_CTRL,HIGH);
  Serial1.begin(115200,SERIAL_8N1,33,34);    
  Serial.println("GPS_test");

  personal.setID(20);
  personal.setup();

  randomSeed((uint32_t)esp_random() ^ (uint32_t)micros());
}

void loop() {
  MF.SetPersonalConst(personal);
  MF.ReceivePacketDevice(personal, st, jitterTargetTime, waitingToSend);
  MF.SendPacketDevice(personal, st, jitterTargetTime, waitingToSend);
}