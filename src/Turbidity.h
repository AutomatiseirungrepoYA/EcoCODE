#ifndef SENSOREN_TURBIDITY_H
#define SENSOREN_TURBIDITY_H

#include <Arduino.h>

class TurbiditySensor {
public:
    // pin       : analog pin the turbidity sensor is on (e.g. A4)
    // threshold : raw ADC value at or above which water is considered clear.
    //             Higher output voltage = clearer water for the common
    //             analog turbidity sensor modules (e.g. SEN0189-style).
    // vRef      : ADC reference voltage. 5.0 for Uno/Nano/Mega, 3.3 for ESP32.
    // adcMax    : Highest ADC value. 1023.0 for 10-bit, 4095.0 for 12-bit.
    TurbiditySensor(uint8_t pin, int threshold, float vRef = 5.0f, float adcMax = 1023.0f);

    void  begin();
    int   read();          // raw ADC value (0-1023)
    bool  isClear();        // true if read() >= threshold
    float readVoltage();    // sensor output voltage
    float readNTU();        // approximate turbidity in NTU (nephelometric turbidity units)
private:
    uint8_t _pin;
    int     _threshold;
    float   _vRef;
    float   _adcMax;
};

#endif
