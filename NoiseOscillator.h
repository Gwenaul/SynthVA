#pragma once
#include <cstdlib>

class NoiseOscillator {
public:
    NoiseOscillator(float sampleRate) {}

    void setFrequency(float freq) {
        // Le bruit blanc n'a pas besoin de fréquence
    }

    float process() {
        return ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f; // [-1, 1]
    }
};
