#pragma once
#include <cmath>
#include <random>

// PolyBLEP Pulse avec largeur d'impulsion variable (PWM)
class PolyBLEP_Pulse {
public:
    PolyBLEP_Pulse(float sampleRate) 
        : sampleRate(sampleRate), phase(0.0f), frequency(440.0f), pulseWidth(0.5f) {
        calculatePhaseIncrement();
    }

    void setFrequency(float freq) {
        frequency = freq;
        calculatePhaseIncrement();
    }

    void setPulseWidth(float width) {
        pulseWidth = std::max(0.01f, std::min(0.99f, width)); // Limiter entre 1% et 99%
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

        float value = (phase < pulseWidth) ? 1.0f : -1.0f;
        
        // Appliquer PolyBLEP aux deux transitions
        value -= poly_blep(phase);
        value -= poly_blep(phase - pulseWidth + (phase < pulseWidth ? 1.0f : 0.0f));
        
        return value;
    }

    void resetPhase() { phase = 0.0f; }

private:
    float sampleRate, frequency, phase, phaseIncrement, pulseWidth;
    void calculatePhaseIncrement() { phaseIncrement = frequency / sampleRate; }
};
