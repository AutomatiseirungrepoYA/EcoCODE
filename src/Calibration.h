#ifndef SENSOREN_CALIBRATION_H
#define SENSOREN_CALIBRATION_H

#include "TDS.h"

// Runs an interactive TDS calibration over Serial.
// Collects data points, fits a polynomial, writes result to
// the sensor via sensor.setCoefficients().
// Returns true on success, false if the user triggers too many invalid inputs.
bool runCalibration(TDSSensor &sensor);

#endif