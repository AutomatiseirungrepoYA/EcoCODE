#ifndef SENSOREN_INTERNAL_POLYFIT_H
#define SENSOREN_INTERNAL_POLYFIT_H

// Fits a polynomial of degree deg to n data points (x[], y[]).
// Coefficients stored in coeffs[] as [a0, a1, a2, ...] where:
//   y ≈ a0 + a1·x + a2·x² + ...
// coeffs must hold at least deg+1 floats. Max supported degree: 3.
void polyFit(float x[], float y[], int n, int deg, float coeffs[]);

#endif