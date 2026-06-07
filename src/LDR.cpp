#include "LDR.h"

LDRSensor::LDRSensor(uint8_t pin, int threshold)
    : _pin(pin), _threshold(threshold)
{}

void LDRSensor::begin() {
    pinMode(_pin, INPUT);
}

int LDRSensor::read() {
    return analogRead(_pin);
}

bool LDRSensor::isClear() {
    return read() >= _threshold;
}