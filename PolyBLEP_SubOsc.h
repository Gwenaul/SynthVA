#pragma once
#include <cmath>
#include <random>

// Sub-oscillateur (une octave en dessous)
class PolyBLEP_SubOsc {
public:
    PolyBLEP_SubOsc(float sampleRate) 
        : sampleRate(sampleRate), phase(0.0f), frequency(440.0f) {
        calculatePhaseIncrement();
    }

    void setFrequency(float freq) {
        frequency = freq * 0.5f; // Une octave en dessous
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

        // Signal carré sub-harmonique
        float value = (phase < 0.5f) ? 1.0f : -1.0f;
        value -= poly_blep(phase);
        
        return value * 0.7f; // Légèrement atténué
    }

    void resetPhase() { phase = 0.0f; }

private:
    float sampleRate, frequency, phase, phaseIncrement;
    void calculatePhaseIncrement() { phaseIncrement = frequency / sampleRate; }
};
