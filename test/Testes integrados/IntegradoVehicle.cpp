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
  return vehicle.hasLocation() && vehicle.getSatValid() && vehicle.getHdop() <= 2.0;
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
  
  bool fixOk = hasFixSimple();
  bool satValid = vehicle.getSatValid();
  uint32_t sats = satValid ? vehicle.getSatValue() : 0;
  vehicle.setHdop();
  
  if (fixOk) {
    vehicle.setLatitude();
    vehicle.setLongitude();
    vehicle.setSpeed();
    vehicle.setCourse();
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

    Serial.println("Valores do Bubble360:");
    Serial.printf("base=%.1f extra=%.1f head=%.1f",
                  baseRadius, maxExtra, headingDeg);

    /*
    ##########################
    ENVIO DE PACOTE DESDE DISPOSITIVO
    ##########################
    */

    if (safetyTimer.isReady()) {
      if (vehicle.hasLocation() && !vehicle.isChannelBusy(SAFETY_CHANNEL)) {
        vehicle.sendSafety();
        Serial.println("Pacote enviado.");
      }
      safetyTimer.reset();
    }

    /*
    ###########################
    PACOTE RECEBIDO DE OUTRO DISPOSITIVO
    ###########################
    */
    static float targetLat = 0.0f;
    static float targetLng = 0.0f;
    static bool  hasTarget = false;

    if (vehicle.receive()) {
        targetLat = vehicle.getReceivedLat();
        targetLng = vehicle.getReceivedLng();
        hasTarget = true;
        Serial.println("Pacote recebido.");
        Serial.printf("Lat: %.6f Lon: %.6f\n", targetLat, targetLng);
    }


    /*
    ###########################
    RETORNA QUAL A DIREÇÃO DA PESSOA EM RELAÇÃO AO CAMINHÃO
    ########################### 
    */

    float angPessoa = vehicle.bearingFromTruckDeg(
      vehicle.getLatitude(), vehicle.getLongitude(),
      targetLat, targetLng
    );
   
    float radiusAtPersona = vehicle.sectionRadiusAtPersona(angPessoa);

    float distanceToPersona = vehicle.calculateDistance(targetLat, targetLng);

    if (distanceToPersona <= radiusAtPersona) {
      Serial.println("ALERTA: Pessoa dentro da zona de perigo!");
    } else {
      Serial.println("Pessoa fora da zona de perigo.");
    }

    /*
    ###########################
    Print com todos os dados para debug
    ###########################
    */
    Serial.println("\n");
    Serial.println("Valores do Veículo:");
    Serial.printf("hdop=%.2f sats=%lu\n lat=%.6f lon=%.6f speed=%.1f km/h course=%.1f deg\n",
                  vehicle.getHdop(), (unsigned long)sats, vehicle.getLatitude(), vehicle.getLongitude(), vehicle.getSpeed(), vehicle.getCourse());

    delay(3000);
}

