#pragma once

class PolyBLEP_Triangle {
public:
    PolyBLEP_Triangle(float sampleRate) 
        : sampleRate(sampleRate), phase(0.0f), frequency(440.0f) {
        calculatePhaseIncrement();
    }

    void setFrequency(float freq) {
        frequency = freq;
        calculatePhaseIncrement();
    }

    // Méthode PolyBLEP pour les transitions
    float poly_blep(float t) {
        if (t < 0.0f || t >= 1.0f) return 0.0f;  // Retourner zéro pour éviter les artefacts
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

        // Calcul de l'onde triangulaire
        float value = 4.0f * fabs(phase - 0.5f) - 1.0f;

        // Appliquer la correction PolyBLEP aux bords de la forme d'onde
        value -= poly_blep(phase); // Application de la correction PolyBLEP

        return value;
    }

    void resetPhase() {
        phase = 0.0f;  // Réinitialiser la phase pour chaque nouvelle note
    }

private:
    float sampleRate;
    float frequency;
    float phase;
    float phaseIncrement;

    void calculatePhaseIncrement() {
        phaseIncrement = frequency / sampleRate;
    }
};
