#include "Turbidity.h"

TurbiditySensor::TurbiditySensor(uint8_t pin, int threshold, float vRef, float adcMax)
    : _pin(pin), _threshold(threshold), _vRef(vRef), _adcMax(adcMax)
{}

void TurbiditySensor::begin() {
    pinMode(_pin, INPUT);
}

int TurbiditySensor::read() {
    return analogRead(_pin);
}

bool TurbiditySensor::isClear() {
    return read() >= _threshold;
}

float TurbiditySensor::readVoltage() {
    return (float)read() * _vRef / _adcMax;
}

// Polynomial fit commonly published for 5V-supplied analog turbidity
// modules: NTU falls to ~0 as output approaches clear-water voltage
// (~4.2V) and rises sharply as the water gets murkier. Only valid at
// _vRef = 5.0 — treat as an approximation, not a calibrated reading.
float TurbiditySensor::readNTU() {
    float v = readVoltage();
    if (v >= 4.2f) return 0.0f;
    float ntu = -1120.4f * v * v + 5742.3f * v - 4352.9f;
    if (ntu < 0.0f) ntu = 0.0f;
    return ntu;
}
