#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <TinyGPSPlus.h>
#include <XPowersLib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// ===================== PINOUT DISPLAY (T-Watch S3 Plus) =====================
static const int TFT_MOSI = 13;
static const int TFT_SCLK = 18;
static const int TFT_CS   = 12;
static const int TFT_DC   = 38;
static const int TFT_RST  = -1;     // NULL
static const int TFT_BL   = 45;

// Backlight PWM (reduz aquecimento)
static const int BL_CH   = 0;
static const int BL_FREQ = 5000;
static const int BL_RES  = 8;       // 0..255

static void backlight(uint8_t level) { ledcWrite(BL_CH, level); }

// ===================== PINOUT GNSS =====================
static const int GPS_RX = 41;   // ESP RX  <- GNSS TX
static const int GPS_TX = 42;   // ESP TX  -> GNSS RX

// ===================== PMU (AXP2101) =====================
static const int I2C_SDA = 10;
static const int I2C_SCL = 11;

#ifndef AXP2101_SLAVE_ADDRESS
#define AXP2101_SLAVE_ADDRESS 0x34
#endif

XPowersAXP2101 PMU;

// ===================== GNSS + Display Objects =====================
HardwareSerial GPSSerial(1);
TinyGPSPlus gps;
Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);

// ===================== Autobaud helper =====================
struct ProbeResult { uint32_t bytes=0, dollars=0, newlines=0; };

static ProbeResult probeBaud(uint32_t baud, uint32_t ms = 900) {
  ProbeResult r;
  GPSSerial.end();
  delay(30);
  GPSSerial.setRxBufferSize(2048);
  GPSSerial.begin(baud, SERIAL_8N1, GPS_RX, GPS_TX);

  uint32_t t0 = millis();
  while (millis() - t0 < ms) {
    while (GPSSerial.available()) {
      uint8_t c = (uint8_t)GPSSerial.read();
      r.bytes++;
      if (c == '$') r.dollars++;
      else if (c == '\n') r.newlines++;
    }
    delay(1);
  }
  return r;
}

static uint32_t autoDetectBaud() {
  const uint32_t bauds[] = {38400, 9600, 19200, 115200};
  uint32_t bestBaud = bauds[0];
  int64_t bestScore = -1;

  Serial.println("\n--- Autobaud GNSS ---");
  for (uint32_t b : bauds) {
    ProbeResult r = probeBaud(b);
    int64_t score = (int64_t)r.dollars * 50 + (int64_t)r.newlines * 20 + (int64_t)r.bytes;
    Serial.printf("baud=%lu -> bytes=%lu '$'=%lu \\n=%lu score=%lld\n",
                  (unsigned long)b, (unsigned long)r.bytes,
                  (unsigned long)r.dollars, (unsigned long)r.newlines,
                  (long long)score);
    if (score > bestScore) { bestScore = score; bestBaud = b; }
  }
  Serial.printf("=> Baud escolhido: %lu\n", (unsigned long)bestBaud);
  return bestBaud;
}

// ===================== GNSS power cycle =====================
static void gnssPowerCycle() {
  // Protege caso PMU falhe
  PMU.disableBLDO1();
  delay(1200);
  PMU.enableBLDO1();
  delay(200);
  Serial.println("GNSS power-cycled (BLDO1 OFF/ON)");
}

// ===================== NMEA sat-in-view (GSV total) =====================
static int satsInView = -1;   // total de satélites em vista (do GSV)
static char nmeaLine[200];
static int  nmeaPos = 0;

static bool startsWith(const char* s, const char* p) {
  while (*p) { if (*s++ != *p++) return false; }
  return true;
}

// Parse muito simples: pega o 4o campo do GSV (total sats in view)
static void parseGSVTotal(const char* line) {
  // Ex: $GPGSV,1,1,00,0*65  -> total = 00
  // Ex: $GBGSV,1,1,04,...  -> total = 04
  if (!(startsWith(line, "$GPGSV") || startsWith(line, "$GNGSV") ||
        startsWith(line, "$GAGSV") || startsWith(line, "$GBGSV"))) {
    return;
  }

  // Split por vírgula e pega campo 4 (index 3 após "$xxGSV")
  // Campos: 0:$xxGSV 1:msgs 2:msg# 3:total 4:...
  int comma = 0;
  int fieldStart = -1;
  char totalBuf[8] = {0};
  int tb = 0;

  for (int i = 0; line[i] && line[i] != '*'; i++) {
    if (line[i] == ',') {
      comma++;
      if (comma == 3) fieldStart = i + 1; // próximo char é início do total
      else if (comma == 4 && fieldStart >= 0) break; // fim do campo total
    } else if (fieldStart >= 0 && comma == 3) {
      if (tb < 7) totalBuf[tb++] = line[i];
    }
  }

  if (tb > 0) {
    int val = atoi(totalBuf);
    satsInView = val;
  }
}

// ===================== UI helpers =====================
static void drawHeader(const char* title) {
  tft.fillRect(0, 0, 240, 30, ST77XX_BLACK);
  tft.setCursor(8, 8);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);
  tft.print(title);
}

static void drawLine(int y, const String& label, const String& value, uint16_t color = ST77XX_WHITE) {
  tft.setTextSize(2);
  tft.setTextColor(color);
  tft.setCursor(8, y);
  tft.print(label);
  tft.print(value);
}

static void drawSmall(int y, const String& s) {
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(8, y);
  tft.print(s);
}

// ===================== Main =====================
static uint32_t lastUI = 0;
static uint32_t lastFixMillis = 0;

void setup() {
  Serial.begin(115200);
  delay(800);

  // CPU menor ajuda aquecimento
  setCpuFrequencyMhz(80);

  // Backlight PWM (bem mais frio)
  ledcSetup(BL_CH, BL_FREQ, BL_RES);
  ledcAttachPin(TFT_BL, BL_CH);
  backlight(40); // 0..255 (ajuste)

  // SPI do display
  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);

  // Display init
  tft.init(240, 240);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextWrap(false);

  drawHeader("GNSS");
  drawSmall(40, "Iniciando PMU...");
  delay(200);

  // PMU
  Wire.begin(I2C_SDA, I2C_SCL);
  bool pmuOk = PMU.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL);
  Serial.printf("PMU begin: %s\n", pmuOk ? "OK" : "FALHOU");

  tft.fillRect(0, 30, 240, 210, ST77XX_BLACK);
  drawHeader("GNSS");

  if (pmuOk) {
    PMU.setBLDO1Voltage(3300);
    PMU.enableBLDO1();
    drawSmall(40, "BLDO1 GNSS: ON (3.3V)");
  } else {
    drawSmall(40, "PMU falhou! GNSS pode OFF");
  }

  delay(300);

  // Autobaud
  uint32_t baud = autoDetectBaud();
  GPSSerial.end();
  delay(30);
  GPSSerial.setRxBufferSize(4096);
  GPSSerial.begin(baud, SERIAL_8N1, GPS_RX, GPS_TX);

  Serial.printf("GPS UART em %lu (RX=%d TX=%d)\n", (unsigned long)baud, GPS_RX, GPS_TX);

  drawSmall(55, "UART OK. Aguarde fix...");
  lastUI = millis();
}

void loop() {
  // ===== Read GNSS =====
  while (GPSSerial.available()) {
    char c = (char)GPSSerial.read();

    // TinyGPS++
    gps.encode(c);

    // NMEA line capture (pra extrair GSV total)
    if (c == '\r') continue;
    if (nmeaPos < (int)sizeof(nmeaLine) - 1) nmeaLine[nmeaPos++] = c;

    if (c == '\n') {
      nmeaLine[nmeaPos] = '\0';
      // remove '\n'
      if (nmeaPos > 0 && nmeaLine[nmeaPos - 1] == '\n') nmeaLine[nmeaPos - 1] = '\0';
      if (nmeaLine[0] == '$') parseGSVTotal(nmeaLine);
      nmeaPos = 0;
    }
  }

  // ===== UI update =====
  if (millis() - lastUI >= 1000) {
    lastUI += 1000;

    const bool fix = gps.location.isValid();
    const int satsUsed = gps.satellites.isValid() ? gps.satellites.value() : 0;
    const float hd = gps.hdop.isValid() ? gps.hdop.hdop() : -1.0f;

    if (fix) lastFixMillis = millis();

    String status = fix ? "FIX OK" : "NO FIX";
    String hdopS  = (hd >= 0) ? String(hd, 2) : "N/A";
    String latS   = fix ? String(gps.location.lat(), 6) : "N/A";
    String lonS   = fix ? String(gps.location.lng(), 6) : "N/A";
    String inView = (satsInView >= 0) ? String(satsInView) : "N/A";

    // clear body area
    tft.fillRect(0, 30, 240, 210, ST77XX_BLACK);
    drawHeader("GNSS");

    // Cores simples pro status
    uint16_t stColor = fix ? ST77XX_GREEN : ST77XX_RED;

    drawLine(40,  "Status: ", status, stColor);
    drawLine(70,  "Used:   ", String(satsUsed));
    drawLine(100, "InView: ", inView);
    drawLine(130, "HDOP:   ", hdopS);

    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(8, 160);
    tft.print("Lat: "); tft.print(latS);

    tft.setCursor(8, 180);
    tft.print("Lon: "); tft.print(lonS);

    // Dica dinâmica
    if (!fix) {
      tft.setCursor(8, 205);
      tft.print("Ceu aberto. Aguarde...");
    }

    // Auto reset GNSS se ficar muito tempo sem ver satélites
    // (só se quiser: evita travas estranhas)
    if (!fix && satsInView == 0) {
      // se passar 120s sem fix, faz um power-cycle do GNSS
      static uint32_t noFixStart = 0;
      if (noFixStart == 0) noFixStart = millis();
      if (millis() - noFixStart > 120000) {
        gnssPowerCycle();
        noFixStart = 0;
      }
    } else {
      // reset timer se começou a ver algo
      static uint32_t noFixStart = 0;
      noFixStart = 0;
    }

    // Serial compacto também
    Serial.printf("fix=%d used=%d inView=%d hdop=%s lat=%s lon=%s\n",
                  fix ? 1 : 0, satsUsed, satsInView,
                  hdopS.c_str(), latS.c_str(), lonS.c_str());
  }
}
