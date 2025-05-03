#pragma once

class PolyBLEP_Square {
public:
    PolyBLEP_Square(float sampleRate) : sampleRate(sampleRate), phase(0.0f), frequency(440.0f) {}

    void setFrequency(float freq) {
        frequency = freq;
        phaseIncrement = frequency / sampleRate;
    }
    
    float process() {
        phase += phaseIncrement;
        if (phase >= 1.0f) phase -= 1.0f;
        return (phase < 0.5f) ? 1.0f : -1.0f; // Simple carré
    }

private:
    float sampleRate;
    float frequency;
    float phase;
    float phaseIncrement;
};
