#include "math_functions.h"

float Pi(int K) {
    float s = 0;
    for (int i = 0; i < K; i++) {
        float t = 1.0 / (2*i + 1);
        if (i % 2) t = -t;
        s += t;
    }
    return 4 * s;
}

float Square(float A, float B) {
    return A * B;
}