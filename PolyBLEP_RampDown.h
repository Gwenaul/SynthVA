#pragma once
#include <cmath>
#include <random>

// Dent de scie inversée (Ramp Down)
class PolyBLEP_RampDown {
public:
    PolyBLEP_RampDown(float sampleRate) 
        : sampleRate(sampleRate), phase(0.0f), frequency(440.0f) {
        calculatePhaseIncrement();
    }

    void setFrequency(float freq) {
        frequency = freq;
        calculatePhaseIncrement();
    }

    float poly_blep(float t) {
        if (t < 0.0f || t >= 1.0f) return 0.0f;
        float dt = phaseIncrement;
        if (t < dt) {
            return t / dt - (t * t) / (2.0f * dt * dt);
        } else if (t > 1.0f - dt) {
            return (1.0f - t) / dt - ((1.0f - t) * (1.0f - t)) / (2.0f * dt * dt);
        }
        return 0.0f;
    }

    float process() {
        phase += phaseIncrement;
        if (phase >= 1.0f) phase -= 1.0f;

        // Dent de scie inversée : descend de 1 à -1
        float value = 1.0f - 2.0f * phase;
        value += poly_blep(phase); // Correction opposée pour la rampe descendante
        
        return value;
    }

    void resetPhase() { phase = 0.0f; }

private:
    float sampleRate, frequency, phase, phaseIncrement;
    void calculatePhaseIncrement() { phaseIncrement = frequency / sampleRate; }
};

