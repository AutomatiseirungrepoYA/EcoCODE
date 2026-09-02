#ifndef SENSOREN_INTERNAL_EEPROM_UTILS_H
#define SENSOREN_INTERNAL_EEPROM_UTILS_H

#include <Arduino.h>

// Magic word — marks a valid calibration block in EEPROM
static const uint16_t CALIB_MAGIC = 0xCA1Bu;

// EEPROM block layout (relative to offset):
//   bytes 0-1  : magic word   (uint16_t)
//   byte  2    : degree       (uint8_t)
//   bytes 3-18 : coefficients (4 x float = 16 bytes)
//   total      : 19 bytes per sensor
static const int EEPROM_BLOCK_SIZE = 19;

// Hands out a fresh, non-overlapping EEPROM region of `size` bytes each
// time it's called, starting at 0. Every sensor that auto-assigns its
// EEPROM offset (TDSSensor, PHSensor, ...) must request its block through
// this allocator so their regions never collide, regardless of type or
// declaration order.
int eeprom_allocateBlock(int size);

// Returns true if a valid calibration exists at offset
bool eeprom_isValid(int offset);

// Loads degree + coefficients from EEPROM. Returns true on success.
bool eeprom_loadCalib(int offset, uint8_t &degree, float coeffs[], uint8_t maxCoeffs);

// Writes degree + coefficients to EEPROM. Magic written last —
// power failure mid-write leaves the old calibration intact.
void eeprom_saveCalib(int offset, uint8_t degree, const float coeffs[], uint8_t numCoeffs);

#endif