#pragma once
#include <random>

class NoiseOscillator {
public:
    enum class NoiseType {
        WHITE,
        PINK,
        BROWN
    };

    NoiseOscillator(float sampleRate, NoiseType noiseType = NoiseType::WHITE)
        : sampleRate(sampleRate), noiseType(noiseType), generator(std::random_device{}()), distribution(-1.0f, 1.0f) {
        // Pour générer du bruit de type Pink ou Brown, des algorithmes spécifiques sont nécessaires
        if (noiseType == NoiseType::PINK) {
            // Initialiser un générateur de bruit rose (méthode basique, mais pas encore implémentée ici)
        }
        else if (noiseType == NoiseType::BROWN) {
            // Initialiser un générateur de bruit marron (méthode basique, mais pas encore implémentée ici)
        }
    }

    void resetPhase() {
    // Pas nécessaire pour le bruit, mais méthode incluse pour uniformiser le comportement
    }

    void setFrequency(float freq) {
        // Le bruit n'a pas de fréquence propre, mais cette fonction peut être utilisée pour changer le type de bruit ou d'autres paramètres.
    }

    void setNoiseType(NoiseType newType) {
    noiseType = newType;
    }

    float process() {
        switch (noiseType) {
            case NoiseType::WHITE:
                return whiteNoise();
            case NoiseType::PINK:
                return pinkNoise();
            case NoiseType::BROWN:
                return brownNoise();
        }
        return 0.0f;  // Ne devrait jamais arriver
    }

private:
    float sampleRate;
    NoiseType noiseType;
    std::mt19937 generator;  // Générateur de nombres pseudo-aléatoires
    std::uniform_real_distribution<float> distribution;  // Distribution uniforme entre [-1, 1]

    float brownLastValue = 0.0f;
    
    // Fonction pour générer du bruit blanc
    float whiteNoise() {
        return distribution(generator);  // Renvoie une valeur aléatoire entre -1 et 1
    }

    // Bruit rose Paul Kellet
    float pinkNoise() {
        static float b0 = 0, b1 = 0, b2 = 0, b3 = 0, b4 = 0, b5 = 0, b6 = 0;
        float white = whiteNoise();
        b0 = 0.99886f * b0 + white * 0.0555179f;
        b1 = 0.99332f * b1 + white * 0.0750759f;
        b2 = 0.96900f * b2 + white * 0.1538520f;
        b3 = 0.86650f * b3 + white * 0.3104856f;
        b4 = 0.55000f * b4 + white * 0.5329522f;
        b5 = -0.7616f * b5 - white * 0.0168980f;
        float pink = b0 + b1 + b2 + b3 + b4 + b5 + b6 + white * 0.5362f;
        b6 = white * 0.115926f;
        return pink * 0.11f; // gain reduction
        }

    // Bruit marron (intégration du bruit blanc pour obtenir une pente de -6 dB/octave)
    float brownNoise() {
        float value = whiteNoise();
        brownLastValue = (brownLastValue * 0.99f) + value * 0.01f;  // Intégration simple
        return brownLastValue * 3.5f;
    }
};
