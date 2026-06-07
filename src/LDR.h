#ifndef SENSOREN_LDR_H
#define SENSOREN_LDR_H

#include <Arduino.h>

class LDRSensor {
public:
    // pin       : analog pin the LDR is on (e.g. A4)
    // threshold : raw ADC value at or above which water is considered clear.
    //             Your original value was 970 — pass that in the constructor.
    LDRSensor(uint8_t pin, int threshold);

    void begin();
    int  read();     // raw ADC value (0–1023)
    bool isClear();  // true if read() >= threshold
private:
    uint8_t _pin;
    int     _threshold;
};

#endif