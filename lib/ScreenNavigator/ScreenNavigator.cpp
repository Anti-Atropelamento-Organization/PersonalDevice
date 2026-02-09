#include "ScreenNavigator.h"

// ===== Button =====
bool ScreenNavigator::Button::readRaw() const {
  int v = digitalRead(pin);
  return activeLow ? (v == LOW) : (v == HIGH);
}

void ScreenNavigator::Button::begin() {
  pinMode(pin, INPUT_PULLUP);
  stable = readRaw();
  lastStable = stable;
  lastChangeMs = millis();
}

void ScreenNavigator::Button::update(uint32_t debounceMs, uint32_t longMs) {
  click = false;
  longPress = false;

  bool raw = readRaw();

  // debounce: só aceita troca após debounceMs
  if (raw != stable) {
    if (millis() - lastChangeMs >= debounceMs) {
      lastStable = stable;
      stable = raw;
      lastChangeMs = millis();

      if (stable && !lastStable) { // pressionou
        downSinceMs = millis();
        longFired = false;
      }

      if (!stable && lastStable) { // soltou
        if (!longFired) click = true;
      }
    }
  } else {
    lastChangeMs = millis();
  }

  // long press: dispara 1x por aperto
  if (stable && !longFired && (millis() - downSinceMs >= longMs)) {
    longPress = true;
    longFired = true;
  }
}

// ===== ScreenNavigator =====
ScreenNavigator::ScreenNavigator(Display &d, int pinBoot, bool activeLow)
  : display(d)
{
  boot.pin = pinBoot;
  boot.activeLow = activeLow;
}

void ScreenNavigator::begin() {
  boot.begin();
  activeScreen = Display::SCREEN_GPS;
  display.setScreen(activeScreen);
}

void ScreenNavigator::toggleScreen() {
  activeScreen = (activeScreen == Display::SCREEN_GPS)
               ? Display::SCREEN_SAFETY
               : Display::SCREEN_GPS;

  display.setScreen(activeScreen);
}

void ScreenNavigator::tick() {
  boot.update(debounceMs, longPressMs);

  if (boot.click) {
    toggleScreen();
  }

  if (boot.longPress) {
    toggleScreen(); // se quiser, posso trocar pra "não faz nada"
  }
}
