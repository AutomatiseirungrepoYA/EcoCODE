#ifndef SENSOREN_TURBIDITY_H
#define SENSOREN_TURBIDITY_H

#include <Arduino.h>

class TurbiditySensor {
public:
    // pin                 : analog pin the turbidity sensor is on (e.g. A4)
    // fullyClearThreshold : raw ADC value at or above which water is
    //                       considered fully clear. Higher output voltage =
    //                       clearer water for the common analog turbidity
    //                       sensor modules (e.g. SEN0189-style); readings
    //                       fall as the water gets more turbid.
    // vRef                : ADC reference voltage. 5.0 for Uno/Nano/Mega, 3.3 for ESP32.
    // adcMax              : Highest ADC value. 1023.0 for 10-bit, 4095.0 for 12-bit.
    //
    // partiallyClearThreshold and fullyDarkThreshold both default to 0 —
    // set them with their setters below once you've observed real readings
    // from your probe. For the three levels to behave as a sensible ladder,
    // set fullyClearThreshold > partiallyClearThreshold > fullyDarkThreshold.
    TurbiditySensor(uint8_t pin, int fullyClearThreshold, float vRef = 5.0f, float adcMax = 1023.0f);

    void  begin();
    int   read();          // raw ADC value (0-1023)
    float readVoltage();   // sensor output voltage

    // Each level is checked against its own threshold independently.
    bool  isFullyClear();       // true if read() >= fullyClearThreshold
    bool  isPartiallyClear();   // true if read() >= partiallyClearThreshold
    bool  isFullyDark();        // true if read() <= fullyDarkThreshold

    void  setFullyClearThreshold(int threshold);       // raw ADC value; in-memory only, not persisted to EEPROM
    int   getFullyClearThreshold() const;
    void  setPartiallyClearThreshold(int threshold);    // raw ADC value; in-memory only, not persisted to EEPROM
    int   getPartiallyClearThreshold() const;
    void  setFullyDarkThreshold(int threshold);         // raw ADC value; in-memory only, not persisted to EEPROM
    int   getFullyDarkThreshold() const;
private:
    uint8_t _pin;
    int     _fullyClearThreshold;
    int     _partiallyClearThreshold;
    int     _fullyDarkThreshold;
    float   _vRef;
    float   _adcMax;
};

#endif
