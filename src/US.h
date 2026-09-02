#ifndef SENSOREN_US_H
#define SENSOREN_US_H

#include <Arduino.h>

// Timing constants (HC-SR04 / compatible)
#define US_TRIG_WIDTH        12      // trigger pulse width in µs
#define US_ROUNDTRIP_CM      57      // µs per cm round-trip
#define US_MAX_DISTANCE     500      // maximum measurable distance in cm
#define US_MAX_SENSOR_DELAY 5800     // µs allowed for sensor to arm
#define US_PING_INTERVAL    30000    // µs between pings in median (30 ms)
#define US_NO_ECHO             0     // returned when no echo is received
#define US_MAX_SAMPLES        15     // hard cap on median sample count

class USSensor {
public:
    // trigPin           : digital pin connected to HC-SR04 TRIG
    // echoPin           : digital pin connected to HC-SR04 ECHO
    // areaCm2           : cross-section area of the container in cm²
    //                     cylinder  → PI * r * r  (r in cm)
    //                     rectangle → length * width  (both in cm)
    // containerHeightCm : total interior height of the container in cm
    // samples           : pings per reading for median filter (default 5)
    USSensor(uint8_t trigPin,
             uint8_t echoPin,
             float   areaCm2,
             float   containerHeightCm,
             uint8_t samples = 5);

    // Call once in setup() to configure pins.
    void  begin();

    // Median-filtered distance from the sensor face to the liquid surface (cm).
    // Returns 0 if no echo is received within US_MAX_DISTANCE.
    float readDistanceCm();

    // Height of liquid measured from the container bottom (cm).
    float fillHeightCm();

    // Fill level as a percentage of the total container height (0 – 100 %).
    float fillPercent();

    // Volume of liquid in litres  (cm³ / 1000).
    float fillLitres();

private:
    uint8_t      _trigPin;
    uint8_t      _echoPin;
    float        _areaCm2;
    float        _heightCm;
    uint8_t      _samples;
    unsigned int _maxEchoTime;

    bool         _pingTrigger();      // fire trigger; return true once echo starts
    unsigned int _ping();             // single raw echo duration in µs
};

#endif
