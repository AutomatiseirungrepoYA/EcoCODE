#include "polyfit.h"
#include <Arduino.h>
#include "sensoren_config.h"

// change:  float M[5][6];
// to:      float M[SENSOREN_MAX_DEGREE + 2][SENSOREN_MAX_DEGREE + 3];

void polyFit(float x[], float y[], int n, int deg, float coeffs[]) {
    int d = deg + 1;
    float M[5][6];

    for (int i = 0; i < d; i++) {
        for (int j = 0; j < d; j++) {
            float s = 0;
            for (int k = 0; k < n; k++) s += pow(x[k], i + j);
            M[i][j] = s;
        }
        float s = 0;
        for (int k = 0; k < n; k++) s += y[k] * pow(x[k], i);
        M[i][d] = s;
    }

    for (int i = 0; i < d; i++) {
        int maxRow = i;
        for (int r = i + 1; r < d; r++)
            if (fabs(M[r][i]) > fabs(M[maxRow][i])) maxRow = r;

        if (maxRow != i)
            for (int c = 0; c <= d; c++) {
                float t = M[i][c]; M[i][c] = M[maxRow][c]; M[maxRow][c] = t;
            }

        float piv = M[i][i];
        if (fabs(piv) < 1e-9f) {
            Serial.println(F("polyFit: singulaere Matrix"));
            for (int k = 0; k < d; k++) coeffs[k] = 0;
            return;
        }

        for (int c = 0; c <= d; c++) M[i][c] /= piv;
        for (int r = 0; r < d; r++) {
            if (r == i) continue;
            float f = M[r][i];
            for (int c = 0; c <= d; c++) M[r][c] -= f * M[i][c];
        }
    }

    for (int i = 0; i < d; i++) coeffs[i] = M[i][d];
}