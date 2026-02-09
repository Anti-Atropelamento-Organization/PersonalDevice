#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

#include "LoRaBoards.h"
#include "DeviceBase.h"
#include "SimpleTimer.h"
#include "vehicleDevice.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include "BLEHandle.h"

unsigned long lastMillis = 0;

SimpleTimer safetyTimer(1000);

VehicleDevice vehicle;

// função que verica se fix ja é true, ou seja, se o GPS ja tem um valor válido
static inline bool hasFixSimple() {
  return vehicle.hasLocation() && vehicle.getSatValid() && vehicle.getSatValue() >= 4;
}


void setup() {
  Serial.begin(115200);
  delay(200);

  setupBoards();
  delay(300);

  vehicle.setID(1);
  vehicle.setup();
  
  randomSeed(digitalRead(0));
  lastMillis = millis();
}


void loop() {


  vehicle.alimentandoGPS();
  
  bool fixOk   = hasFixSimple();
  bool satValid = vehicle.getSatValid();
  uint32_t sats = satValid ? vehicle.getSatValue() : 0;
  
  if (fixOk) {
    vehicle.setLatitude();
    vehicle.setLongitude();
    vehicle.setSpeed();
    vehicle.setCourse();
    vehicle.setHdop();
    vehicle.setRadius(vehicle.getHdop());
  }

/* ==========================================
Implementação do buildDynamicBubble360
 ========================================== */

 // Depois substituir pela orientação da bussula digital
 float headingDeg = 25.0f;
 float baseRadius = (float)vehicle.getRadius(2);
 float v_kmh = (float)vehicle.getSpeed();
 float maxExtra = v_kmh * 0.6f;

  if (maxExtra < 0.0f) maxExtra = 0.0f;
  if (maxExtra > 80.0f) maxExtra = 80.0f;

   int qntdSetores = 45;

   // exponent escolhido através de testes visuais feitos com um html
   float exponent = 3.0f;

   vehicle.buildDynamicBubble360(
      headingDeg,
      baseRadius,
      maxExtra,
      qntdSetores,
      exponent
    );

    Serial.printf("HDOP=%.2f base=%.1f v=%.1f extra=%.1f head=%.1f sats=%lu\n",
                  (float)vehicle.getHdop(), baseRadius, v_kmh, maxExtra, headingDeg,
                  (unsigned long)sats);
}

