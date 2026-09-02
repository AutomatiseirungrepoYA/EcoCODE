#include "US.h"

// ── Constructor ───────────────────────────────────────────────────────────────

USSensor::USSensor(uint8_t trigPin, uint8_t echoPin,
                   float areaCm2, float containerHeightCm, uint8_t samples)
    : _trigPin(trigPin),
      _echoPin(echoPin),
      _areaCm2(areaCm2),
      _heightCm(containerHeightCm),
      _samples(samples)
{
    if (_samples == 0 || _samples > US_MAX_SAMPLES) _samples = 5;
    _maxEchoTime = (unsigned int)((US_MAX_DISTANCE + 1UL) * US_ROUNDTRIP_CM);
}

// ── Public ────────────────────────────────────────────────────────────────────

void USSensor::begin() {
    pinMode(_trigPin, OUTPUT);
    digitalWrite(_trigPin, LOW);
    pinMode(_echoPin, INPUT);
}

float USSensor::readDistanceCm() {
    // Collect up to _samples valid pings and keep them insertion-sorted.
    // Returns median. Skips out-of-range pings (US_NO_ECHO).
    uint8_t      it = _samples;
    unsigned int uS[US_MAX_SAMPLES];
    uint8_t j, i = 0;
    uS[0] = US_NO_ECHO;

    while (i < it) {
        unsigned long t    = micros();
        unsigned int  last = _ping();

        if (last != US_NO_ECHO) {
            // Insertion sort descending so median is uS[it >> 1]
            if (i > 0) {
                for (j = i; j > 0 && uS[j - 1] < last; j--)
                    uS[j] = uS[j - 1];
            } else {
                j = 0;
            }
            uS[j] = last;
            i++;
        } else {
            it--;   // out-of-range: shrink the target count
        }

        // Enforce minimum 30 ms gap between pings
        if (i < it && micros() - t < US_PING_INTERVAL)
            delay((US_PING_INTERVAL + t - micros()) >> 10);
    }

    if (it == 0) return 0.0f;   // every ping timed out → no echo
    return (float)(uS[it >> 1] / US_ROUNDTRIP_CM);
}

float USSensor::fillHeightCm() {
    float dist = readDistanceCm();
    if (dist <= 0.0f) return 0.0f;
    float h = _heightCm - dist;
    if (h < 0.0f)       h = 0.0f;
    if (h > _heightCm)  h = _heightCm;
    return h;
}

float USSensor::fillPercent() {
    if (_heightCm <= 0.0f) return 0.0f;
    return (fillHeightCm() / _heightCm) * 100.0f;
}

float USSensor::fillLitres() {
    return (_areaCm2 * fillHeightCm()) / 1000.0f;
}

// ── Private helpers ───────────────────────────────────────────────────────────

bool USSensor::_pingTrigger() {
    // Send 12 µs trigger pulse
    digitalWrite(_trigPin, HIGH);
    delayMicroseconds(US_TRIG_WIDTH);
    digitalWrite(_trigPin, LOW);

    // Bail if a previous echo is still active
    if (digitalRead(_echoPin)) return false;

    // Wait for the sensor to arm and echo pin to go HIGH
    unsigned long deadline = micros() + _maxEchoTime + US_MAX_SENSOR_DELAY;
    while (!digitalRead(_echoPin)) {
        if (micros() > deadline) return false;
    }
    return true;
}

unsigned int USSensor::_ping() {
    if (!_pingTrigger()) return US_NO_ECHO;

    unsigned long echoStart = micros();
    unsigned long deadline  = echoStart + _maxEchoTime;

    // Measure how long echo pin stays HIGH
    while (digitalRead(_echoPin)) {
        if (micros() > deadline) return US_NO_ECHO;
    }
    return (unsigned int)(micros() - echoStart);
}
