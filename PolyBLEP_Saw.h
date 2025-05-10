#pragma once
#include <cmath>

class PolyBLEP_Saw {
public:
    float phase = 0.0f;
    float freq = 440.0f;
    float sampleRate = 44100.0f;

    PolyBLEP_Saw(float sr) : sampleRate(sr) {}

    // Fonction PolyBLEP améliorée
    float poly_blep(float t) {
        if (t < 0.0f || t >= 1.0f) return 0.0f;  // Retour à zéro pour éviter les artefacts
        float dt = sampleRate / freq;
        float dt2 = dt * dt;
        if (t < dt) {
            // Transition au début de la période
            return t / dt - (t * t) / (2.0f * dt2);
        } else if (t > 1.0f - dt) {
            // Transition à la fin de la période
            float dt2 = dt * dt;
            return (1.0f - t) / dt - ((1.0f - t) * (1.0f - t)) / (2.0f * dt2);
        }
        return 0.0f;
    }

    float process() {
        float dt = freq / sampleRate;
        phase += dt;
        if (phase >= 1.0f) phase -= 1.0f;

        // Calcul de la dent de scie
        float value = 2.0f * phase - 1.0f;

        // Application de la correction PolyBLEP
        value -= poly_blep(phase);

        return value;
    }

    void resetPhase() {
        phase = 0.0f; // Réinitialisation de la phase à chaque note
    }

    void setFrequency(float newFreq) {
        freq = newFreq;
    }
};
