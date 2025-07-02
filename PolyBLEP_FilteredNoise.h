#pragma once
#include <cmath>
#include <random>

class PolyBLEP_FilteredNoise {
public:
    PolyBLEP_FilteredNoise(float sampleRate) 
        : sampleRate(sampleRate), gen(std::random_device{}()), dis(-1.0f, 1.0f) {
        // Filtre passe-bas simple pour adoucir le bruit
        lpf_a = exp(-2.0f * M_PI * 800.0f / sampleRate); // Fréquence de coupure à 800Hz
        lpf_b = 1.0f - lpf_a;
        lpf_y = 0.0f;
    }

    // Constructeur de copie
    PolyBLEP_FilteredNoise(const PolyBLEP_FilteredNoise& other)
        : sampleRate(other.sampleRate), gen(std::random_device{}()), 
          dis(other.dis), lpf_a(other.lpf_a), lpf_b(other.lpf_b), lpf_y(other.lpf_y) {}

    // Opérateur d'assignation
    PolyBLEP_FilteredNoise& operator=(const PolyBLEP_FilteredNoise& other) {
        if (this != &other) {
            sampleRate = other.sampleRate;
            gen.seed(std::random_device{}()); // Nouvelle seed
            dis = other.dis;
            lpf_a = other.lpf_a;
            lpf_b = other.lpf_b;
            lpf_y = other.lpf_y;
        }
        return *this;
    }

    float process() {
        float noise = dis(gen);
        lpf_y = lpf_a * lpf_y + lpf_b * noise; // Filtre passe-bas
        return lpf_y;
    }

    void resetPhase() {
        lpf_y = 0.0f; // Reset du filtre
    }

private:
    float sampleRate;
    std::mt19937 gen; // Pas de random_device stocké
    std::uniform_real_distribution<float> dis;
    float lpf_a, lpf_b, lpf_y; // Coefficients du filtre passe-bas
};