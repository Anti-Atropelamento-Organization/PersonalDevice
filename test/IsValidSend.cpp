#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

#include "LoRaBoards.h"
#include "VehicleDevice.h"
#include "PersonalDevice.h"
#include "SimpleTimer.h"

#include "Display.h"
#include "ScreenNavigator.h"

// ===== T-BEAM S3 CORE =====
static const int PIN_BOOT  = 0;

// ===== I2C do OLED =====
static const int OLED_SDA = 17;
static const int OLED_SCL = 18;

VehicleDevice vehicle;
PersonalDevice personal;

SimpleTimer safetyTimer(5000);

// SH1106 via HW I2C
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

Display display(oled);

ScreenNavigator nav(display, PIN_BOOT, true);

static inline bool hasFixSimple() {
  return personal.hasLocation() && personal.getSatValid() && personal.getSatValue() >= 4;
}

void setup() {
  Serial.begin(115200);
  delay(200);

  setupBoards();
  delay(300);

  Wire.begin(OLED_SDA, OLED_SCL);

  oled.begin();
  oled.setFont(u8g2_font_6x12_tf);

  display.begin();
  display.setRefreshMs(200);

  nav.begin();

  display.setGpsStatus(false, 0, false, 0, 0, 0, 0);
}

void loop() {
  personal.alimentandoGPS();

  bool fixOk   = hasFixSimple();
  bool satValid = personal.getSatValid();
  uint32_t sats = satValid ? personal.getSatValue() : 0;

  display.setGpsStatus(
    satValid, sats, fixOk,
    personal.getLatitude(), personal.getLongitude(),
    personal.getHdop(), personal.getSpeed()
  );

  static float targetLat = 0.0f;
  static float targetLng = 0.0f;
  static bool  hasTarget = false;

  if (personal.receive()) {
    targetLat = personal.getReceivedLat();
    targetLng = personal.getReceivedLng();
    hasTarget = true;
  }

  int level = 3;
  if (hasTarget) {
    level = personal.isValidSend(targetLat, targetLng); 
  }

  static int lastLevel = -1;
  if (level != lastLevel) {
    if (level == 1) safetyTimer.setInterval(3000);
    else if (level == 2) safetyTimer.setInterval(10000);
    else if (level == 3) safetyTimer.setInterval(120000);
    safetyTimer.reset();
    lastLevel = level;
    Serial.println("Nível de alerta atualizado: " + String(level));
  }


  if ((level != -1) && safetyTimer.isReady()) {

    if (personal.hasLocation() && !vehicle.isChannelBusy(SAFETY_CHANNEL)) {
      vehicle.sendSafety();  
    }
    safetyTimer.reset();
  }


  nav.tick();
  display.tick();
}

