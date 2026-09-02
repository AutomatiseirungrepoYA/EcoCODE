#ifndef SENSOREN_DS18B20_H
#define SENSOREN_DS18B20_H

#include <Arduino.h>
#include <OneWire.h>

class DS18B20Sensor {
public:
    // pin : digital pin the DS18B20 data line is on. Needs a 4.7k pull-up
    //       resistor between data and VCC (unless the module has one built in).
    DS18B20Sensor(uint8_t pin);

    // Call once in setup() — finds the sensor on the bus and validates its
    // address. isFound() returns false afterwards if none was detected.
    void  begin();
    bool  isFound() const;

    // Triggers a conversion and returns the temperature in Celsius.
    // Blocks for ~750ms (worst case at 12-bit resolution).
    // Returns -127.0 if no sensor was found or the reading failed a CRC check.
    float read();
private:
    uint8_t _pin;
    OneWire _wire;
    uint8_t _addr[8];
    bool    _found;
};

#endif
