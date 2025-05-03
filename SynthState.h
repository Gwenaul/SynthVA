#ifndef SYNTHSTATE_H
#define SYNTHSTATE_H

#include <vector>
#include "Voice.h"
#include "ADSR.h"
#include "MoogFilter.h"
#include "Waveform.h"
#include "PolyBLEP_Saw.h"
#include "PolyBLEP_Square.h"
#include "PolyBLEP_Triangle.h"
#include "NoiseOscillator.h"
#include "LFO.h"

struct SynthState {
    // Oscillateurs
    PolyBLEP_Saw* osc;
    PolyBLEP_Square* oscSquare;
    PolyBLEP_Triangle* oscTriangle;
    NoiseOscillator* noiseOsc;

    const int maxVoices = 8;
    std::vector<Voice> voices;

    Waveform* waveform = nullptr;

    // Paramètres contrôlables
    float volume = 0.8f;
    int octave = 4;
    bool noteOn = false;

    // Fréquence d'échantillonnage
    float sampleRate;

    // Enveloppe ADSR globale (utilisée comme modèle pour les nouvelles voix)
    ADSR env;
    
    // Paramètres ADSR (stockés au niveau du SynthState pour les nouvelles voix)
    float attackTime = 0.05f;
    float decayTime = 0.9f;
    float sustainLevel = 0.9f;
    float releaseTime = 0.9f;

    // Filtre
    MoogFilter moogFilter;

    // Mix des oscillateurs
    float mixSaw = 1.0f;
    float mixSquare = 0.0f;
    float mixTriangle = 0.0f;
    float mixNoise = 0.0f;

    // LFO
    float lfoFrequency = 5.0f;
    float lfoDepth = 10.0f;
    LFO lfo;
    bool lfoEnabled = false;

    // Constructeur
    SynthState(float sampleRate)
        : sampleRate(sampleRate), env(sampleRate), moogFilter(sampleRate), lfo(sampleRate)
    {
        lfo.setFrequency(lfoFrequency);
    }

    // Nettoyer les voix inactives périodiquement
    void cleanupVoices() {
        auto it = voices.begin();
        while (it != voices.end()) {
            if (!it->isActive()) {
                it = voices.erase(it);
            } else {
                ++it;
            }
        }
    }
};

#endif // SYNTHSTATE_H