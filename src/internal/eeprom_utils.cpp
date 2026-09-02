#include "eeprom_utils.h"
#include <EEPROM.h>

static int _nextEepromOffset = 0;

int eeprom_allocateBlock(int size) {
    int offset = _nextEepromOffset;
    _nextEepromOffset += size;
    return offset;
}

bool eeprom_isValid(int offset) {
    uint16_t magic = 0;
    EEPROM.get(offset, magic);
    return magic == CALIB_MAGIC;
}

bool eeprom_loadCalib(int offset, uint8_t &degree, float coeffs[], uint8_t maxCoeffs) {
    if (!eeprom_isValid(offset)) return false;

    EEPROM.get(offset + (int)sizeof(uint16_t), degree);

    float buf[4] = {0, 0, 0, 0};
    EEPROM.get(offset + (int)sizeof(uint16_t) + (int)sizeof(uint8_t), buf);

    uint8_t n = min((uint8_t)(degree + 1), maxCoeffs);
    for (uint8_t i = 0; i < n; i++) coeffs[i] = buf[i];

    return true;
}

void eeprom_saveCalib(int offset, uint8_t degree, const float coeffs[], uint8_t numCoeffs) {
    float buf[4] = {0, 0, 0, 0};
    uint8_t n = min(numCoeffs, (uint8_t)4);
    for (uint8_t i = 0; i < n; i++) buf[i] = coeffs[i];

    EEPROM.put(offset + (int)sizeof(uint16_t),                         degree);
    EEPROM.put(offset + (int)sizeof(uint16_t) + (int)sizeof(uint8_t), buf);
    EEPROM.put(offset,                                                 (uint16_t)CALIB_MAGIC);
}