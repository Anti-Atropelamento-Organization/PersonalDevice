#include "SimpleTimer.h"

// Construtor
SimpleTimer::SimpleTimer(unsigned long intervalMs) {
    _interval = intervalMs;
    _lastTime = millis();
}

bool SimpleTimer::isReady() {
    return (millis() - _lastTime >= _interval);
}

void SimpleTimer::reset() {
    _lastTime = millis();
}

void SimpleTimer::setInterval(unsigned long intervalMs) {
    _interval = intervalMs;
}

unsigned long SimpleTimer::getInterval() {
    return _interval;
}