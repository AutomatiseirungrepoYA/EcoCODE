#include "Turbidity.h"

TurbiditySensor::TurbiditySensor(uint8_t pin, int fullyClearThreshold, float vRef, float adcMax)
    : _pin(pin), _fullyClearThreshold(fullyClearThreshold),
      _partiallyClearThreshold(0), _fullyDarkThreshold(0),
      _vRef(vRef), _adcMax(adcMax)
{}

void TurbiditySensor::begin() {
    pinMode(_pin, INPUT);
}

int TurbiditySensor::read() {
    return analogRead(_pin);
}

float TurbiditySensor::readVoltage() {
    return (float)read() * _vRef / _adcMax;
}

bool TurbiditySensor::isFullyClear() {
    return read() >= _fullyClearThreshold;
}

bool TurbiditySensor::isPartiallyClear() {
    return read() >= _partiallyClearThreshold;
}

bool TurbiditySensor::isFullyDark() {
    return read() <= _fullyDarkThreshold;
}

void TurbiditySensor::setFullyClearThreshold(int threshold) {
    _fullyClearThreshold = threshold;
}

int TurbiditySensor::getFullyClearThreshold() const {
    return _fullyClearThreshold;
}

void TurbiditySensor::setPartiallyClearThreshold(int threshold) {
    _partiallyClearThreshold = threshold;
}

int TurbiditySensor::getPartiallyClearThreshold() const {
    return _partiallyClearThreshold;
}

void TurbiditySensor::setFullyDarkThreshold(int threshold) {
    _fullyDarkThreshold = threshold;
}

int TurbiditySensor::getFullyDarkThreshold() const {
    return _fullyDarkThreshold;
}
