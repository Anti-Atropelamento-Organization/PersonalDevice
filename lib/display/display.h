#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
#include "packet.h"

class Display {
public:
  explicit Display(U8G2 &d);

  void begin();

  void setRefreshMs(uint32_t ms);
  void tick();

  enum Screen : uint8_t { SCREEN_GPS, SCREEN_SAFETY };
  void setScreen(Screen s);

  // Alimenta dados
  void setSafety(const SafetyData &d);

  // GPS status
  void setGpsStatus(bool satValid, uint32_t sats, bool fixOk,
                    double lat, double lng, float hdop, float speed);

private:
  U8G2 &oled;

  uint32_t refreshMs = 200;
  uint32_t lastDisp = 0;

  Screen current = SCREEN_GPS;

  // caches
  SafetyData sData{};

  struct {
    bool satValid = false;
    uint32_t sats = 0;
    bool fixOk = false;
    double lat = 0;
    double lng = 0;
    float hdop = 0;
    float speed = 0;
  } gps;

  // controle do “último pacote recebido” (SAFETY)
  uint32_t lastSafetyRxMs = 0;
  uint32_t safetyRxCount  = 0;
  uint8_t  lastSafetySenderId = 0;

  void drawGpsScreen();
  void drawSafetyScreen();
};
