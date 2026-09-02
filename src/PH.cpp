#include "PH.h"

// Fixed calibration: pH = (V - 3.7076) / -0.1666, V = probe voltage.
#define PH_SLOPE  -0.1666f
#define PH_OFFSET  3.7076f
#define PH_CAL_TEMP_C 25.0f  // reference temperature the fixed curve above was fit at

PHSensor::PHSensor(uint8_t pin, float vRef, float adcMax)
    : _pin(pin), _vRef(vRef), _adcMax(adcMax)
{
}

void PHSensor::begin() {
    pinMode(_pin, INPUT);
}

float PHSensor::sampleVoltage() const {
    const int SAMPLES = 20;
    int buf[SAMPLES];
    for (int i = 0; i < SAMPLES; i++) {
        buf[i] = analogRead(_pin);
        delay(10);
    }
    for (int i = 1; i < SAMPLES; i++) {
        int key = buf[i], j = i - 1;
        while (j >= 0 && buf[j] > key) { buf[j + 1] = buf[j]; j--; }
        buf[j + 1] = key;
    }
    int med = buf[SAMPLES / 2];
    return (float)med * _vRef / _adcMax;
}

float PHSensor::read(float tempC) {
    float voltage = sampleVoltage();

    // Correct direction: slope shrinks in magnitude as temperature rises
    // above the reference temperature the fixed curve was fit at. Ratio is
    // T_reference / T_measured, both in Kelvin.
    float tempFactor = (273.15f + PH_CAL_TEMP_C) / (273.15f + tempC);

    // Offset is NOT temperature-compensated — no established compensation
    // model applied here, same known gap as before.
    return (voltage - PH_OFFSET) / (PH_SLOPE * tempFactor);
}
