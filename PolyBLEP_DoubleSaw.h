#pragma once
#include <cmath>
#include <random>

// Double Dent de Scie (Saw avec harmonique supplémentaire)
class PolyBLEP_DoubleSaw {
public:
    PolyBLEP_DoubleSaw(float sampleRate) 
        : sawOsc1(sampleRate), sawOsc2(sampleRate) {}

    void setFrequency(float freq) {
        sawOsc1.setFrequency(freq);
        sawOsc2.setFrequency(freq * 1.01f); // Légèrement désaccordé pour un effet "gras"
    }

    float process() {
        float saw1 = sawOsc1.process();
        float saw2 = sawOsc2.process();
        return (saw1 + saw2 * 0.8f) * 0.6f; // Mix avec atténuation
    }

    void resetPhase() {
        sawOsc1.resetPhase();
        sawOsc2.resetPhase();
    }

private:
    PolyBLEP_Saw sawOsc1, sawOsc2; // Réutilise votre classe existante
};
