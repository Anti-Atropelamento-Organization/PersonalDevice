#include "Display.h"

Display::Display(U8G2 &d) : oled(d) {}

void Display::begin() {
  oled.setFont(u8g2_font_6x12_tf);
}

void Display::setRefreshMs(uint32_t ms) { refreshMs = ms; }
void Display::setScreen(Screen s) { current = s; }

void Display::setSafety(const SafetyData &d) {
  sData = d;

  // registra “último pacote recebido”
  lastSafetyRxMs = millis();
  lastSafetySenderId = sData.ID;
  safetyRxCount++;
}

void Display::setGpsStatus(bool satValid, uint32_t sats, bool fixOk,
                           double lat, double lng, float hdop, float speed) {
  gps.satValid = satValid;
  gps.sats = sats;
  gps.fixOk = fixOk;
  gps.lat = lat;
  gps.lng = lng;
  gps.hdop = hdop;
  gps.speed = speed;
}

void Display::tick() {
  if (millis() - lastDisp < refreshMs) return;
  lastDisp = millis();

  switch (current) {
    case SCREEN_GPS:    drawGpsScreen(); break;
    case SCREEN_SAFETY: drawSafetyScreen(); break;
  }
}

// -------------------- TELAS --------------------

void Display::drawGpsScreen() {
  oled.clearBuffer();
  oled.setFont(u8g2_font_6x12_tf);

  if (!gps.fixOk) {
    oled.drawStr(0, 12, "TSA GPS");
    oled.drawStr(0, 28, "Procurando sinal GPS...");

    char line2[32];
    if (gps.satValid) snprintf(line2, sizeof(line2), "SAT: %lu", (unsigned long)gps.sats);
    else             snprintf(line2, sizeof(line2), "SAT: --");
    oled.drawStr(0, 44, line2);

    char line3[32];
    snprintf(line3, sizeof(line3), "Tempo: %lus", (unsigned long)(millis() / 1000));
    oled.drawStr(0, 60, line3);

    oled.sendBuffer();
    return;
  }

  oled.drawStr(0, 12, "GPS OK");

  char l1[32], l2[32];
  snprintf(l1, sizeof(l1), "Lat: %.6f", gps.lat);
  snprintf(l2, sizeof(l2), "Lng: %.6f", gps.lng);
  oled.drawStr(0, 28, l1);
  oled.drawStr(0, 40, l2);

  char l3[32], l4[32];
  snprintf(l3, sizeof(l3), "HDOP: %.2f  SAT:%lu", gps.hdop, (unsigned long)(gps.satValid ? gps.sats : 0));
  snprintf(l4, sizeof(l4), "Speed: %.2f", gps.speed);
  oled.drawStr(0, 52, l3);
  oled.drawStr(0, 64, l4);

  oled.sendBuffer();
}

void Display::drawSafetyScreen() {
  oled.clearBuffer();
  oled.setFont(u8g2_font_6x12_tf);

  oled.drawStr(0, 12, "SAFETY");

  // header: ultimo pacote recebido + id remetente + tempo
  char header[32];
  uint32_t agoS = (lastSafetyRxMs == 0) ? 0 : (millis() - lastSafetyRxMs) / 1000;

  // Ultimo pacote recebido = SAFETY
  snprintf(header, sizeof(header), "Ult:SAFETY ID:%u", (unsigned)lastSafetySenderId);
  oled.drawStr(0, 24, header);

  char h2[32];
  snprintf(h2, sizeof(h2), "Rx ha: %lus  Cnt:%lu", (unsigned long)agoS, (unsigned long)safetyRxCount);
  oled.drawStr(0, 36, h2);

  // dados do pacote
  float lat = sData.lat / 1000000.0f;
  float lng = sData.lng / 1000000.0f;

  char l1[32], l2[32], l3[32];
  snprintf(l1, sizeof(l1), "Lat: %.6f", lat);
  snprintf(l2, sizeof(l2), "Lng: %.6f", lng);
  snprintf(l3, sizeof(l3), "Spd: %.1f  Crs: %.0f", sData.speed, sData.course);

  oled.drawStr(0, 48, l1);
  oled.drawStr(0, 58, l2);
  oled.drawStr(0, 64, l3);

  oled.sendBuffer();
}
