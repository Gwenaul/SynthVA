#pragma once
#include <cmath>

class LFO {
public:
    float phase;
    float freq;
    float sampleRate;

    LFO(float sr) : phase(0.0f), freq(1.0f), sampleRate(sr) {}

    void setFrequency(float f) { freq = f; }

    float process() {
        float value = std::sin(2.0f * M_PI * phase);
        phase += freq / sampleRate;
        if (phase >= 1.0f) phase -= 1.0f;
        return value;
    }
};
