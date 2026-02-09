#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

#include "LoRaBoards.h"
#include "DeviceBase.h"
#include "VehicleDevice.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include "BLEHandle.h"

#include "mainFunctions.h"
#include "esp_system.h"
#include "SimpleTimer.h"

#define VGNSS_CTRL 3

mainFunctions MF;
VehicleDevice vehicle;

SimpleTimer st(3000);

unsigned long jitterTargetTime = 0;
bool waitingToSend = false;

bool hasTarget = false;

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(VGNSS_CTRL,OUTPUT);
  digitalWrite(VGNSS_CTRL,HIGH);
  Serial1.begin(115200,SERIAL_8N1,33,34);    
  Serial.println("GPS_test");

  vehicle.setID(20);
  vehicle.setup();

  randomSeed((uint32_t)esp_random() ^ (uint32_t)micros());
}

void loop() {
  MF.SetVehicleConst(vehicle);
  MF.ReceivePacketDevice(vehicle, st, jitterTargetTime, waitingToSend, hasTarget);
  MF.SendPacketDevice(vehicle, st, jitterTargetTime, waitingToSend);
}