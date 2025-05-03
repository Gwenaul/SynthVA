// PolyBLEP_Saw.h
#pragma once
#include <cmath>

class PolyBLEP_Saw {
public:
    float phase = 0.0f;
    float freq = 440.0f;
    float sampleRate = 44100.0f;

    PolyBLEP_Saw(float sr) : sampleRate(sr) {}

    float poly_blep(float t) {
        if (t < 0.0f) return 0.0f;
        if (t < 1.0f) {
            float dt = t;
            return dt - (dt * dt / 2.0f);
        }
        return 0.0f;
    }

    float process() {
        float dt = freq / sampleRate;
        phase += dt;
        if (phase >= 1.0f) phase -= 1.0f;

        float value = 2.0f * phase - 1.0f;
        value -= poly_blep(phase / dt);

        return value;
    }

    void setFrequency(float newFreq) {
        freq = newFreq;
    }

};