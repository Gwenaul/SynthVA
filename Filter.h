// Filter.h
#pragma once
#include <cmath>

class ZDF_OnePole {
public:
    float sampleRate;
    float cutoff;
    float resonance;
    float z1 = 0.0f;

    ZDF_OnePole(float sr) : sampleRate(sr), cutoff(1000.0f), resonance(0.0f) {}

    void setCutoff(float freq) {
        cutoff = freq;
    }

    void setResonance(float reso) {
        resonance = reso;
    }

    float process(float input) {
        float T = 1.0f / sampleRate;
        float wd = 2.0f * M_PI * cutoff;
        float wa = (2.0f / T) * tan(wd * T / 2.0f);
        float g = wa * T / 2.0f;

        float R = resonance;
        float S = (input - R * z1) / (1.0f + g);
        float v = g * S;
        float y = v + z1;
        z1 = v + y;

        return y;

    }

};