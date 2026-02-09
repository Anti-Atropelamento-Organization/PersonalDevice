#pragma once
#include <Arduino.h>
#include "Display.h"

class ScreenNavigator {
public:
  // pinBoot: normalmente 0 no T-Beam S3 Core
  ScreenNavigator(Display &d, int pinBoot, bool activeLow = true);

  void begin();
  void tick();  // chame no loop()

  void setDebounceMs(uint32_t ms)  { debounceMs = ms; }
  void setLongPressMs(uint32_t ms) { longPressMs = ms; }

private:
  struct Button {
    int pin = -1;
    bool activeLow = true;

    bool stable = false;
    bool lastStable = false;
    uint32_t lastChangeMs = 0;

    bool click = false;      // clique curto ao soltar
    bool longPress = false;  // dispara 1x quando segurar

    uint32_t downSinceMs = 0;
    bool longFired = false;

    void begin();
    void update(uint32_t debounceMs, uint32_t longMs);
    bool readRaw() const;
  };

  Display &display;
  Button boot;

  uint32_t debounceMs  = 30;
  uint32_t longPressMs = 800;

  Display::Screen activeScreen = Display::SCREEN_GPS;

  void toggleScreen();
};
