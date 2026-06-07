#ifndef SENSOREN_INTERNAL_MEDIAN_H
#define SENSOREN_INTERNAL_MEDIAN_H

// Returns the median of arr[0..len-1]. Sorts a copy — original untouched.
// len must not exceed 20.
inline int medianOf(int arr[], int len) {
    int tmp[20];
    for (int i = 0; i < len; i++) tmp[i] = arr[i];
    for (int j = 0; j < len - 1; j++)
        for (int i = 0; i < len - j - 1; i++)
            if (tmp[i] > tmp[i + 1]) {
                int t = tmp[i]; tmp[i] = tmp[i + 1]; tmp[i + 1] = t;
            }
    return (len & 1) ? tmp[len / 2] : (tmp[len / 2] + tmp[len / 2 - 1]) / 2;
}

#endif