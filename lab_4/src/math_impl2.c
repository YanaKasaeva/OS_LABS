#include "math_functions.h"

float Pi(int K) {
    float p = 1.0;
    for (int i = 1; i <= K; i++) {
        p *= (4.0*i*i) / (4.0*i*i - 1);
    }
    return 2 * p;
}

float Square(float A, float B) {
    return 0.5 * A * B;
}