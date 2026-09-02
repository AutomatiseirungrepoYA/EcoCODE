#include "DS18B20.h"

#define DS18B20_CONVERT_T     0x44
#define DS18B20_READ_SCRATCH  0xBE

DS18B20Sensor::DS18B20Sensor(uint8_t pin)
    : _pin(pin), _wire(pin), _found(false)
{}

void DS18B20Sensor::begin() {
    _wire.reset_search();
    _found = _wire.search(_addr) && (OneWire::crc8(_addr, 7) == _addr[7]);
}

bool DS18B20Sensor::isFound() const {
    return _found;
}

float DS18B20Sensor::read() {
    if (!_found) return -127.0f;

    _wire.reset();
    _wire.select(_addr);
    _wire.write(DS18B20_CONVERT_T, 1);   // parasite-power friendly: keep line driven high

    delay(750);

    _wire.reset();
    _wire.select(_addr);
    _wire.write(DS18B20_READ_SCRATCH);

    uint8_t data[9];
    for (uint8_t i = 0; i < 9; i++) data[i] = _wire.read();

    if (OneWire::crc8(data, 8) != data[8]) return -127.0f;

    int16_t raw = (data[1] << 8) | data[0];
    return (float)raw / 16.0f;
}
