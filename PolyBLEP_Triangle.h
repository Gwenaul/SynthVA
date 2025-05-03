#pragma once

class PolyBLEP_Triangle {
public:
    PolyBLEP_Triangle(float sampleRate) : sampleRate(sampleRate), phase(0.0f), frequency(440.0f) {}

    void setFrequency(float freq) {
        frequency = freq;
        phaseIncrement = frequency / sampleRate;
    }
    
    float process() {
        phase += phaseIncrement;
        if (phase >= 1.0f) phase -= 1.0f;
        
        // Forme triangle basique : dérivée d'une onde carrée intégrée
        float value = 4.0f * fabs(phase - 0.5f) - 1.0f;
        return value;
    }

private:
    float sampleRate;
    float frequency;
    float phase;
    float phaseIncrement;
};
