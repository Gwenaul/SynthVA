#ifndef SYNTHSTATE_H
#define SYNTHSTATE_H

#include <vector>
#include "Voice.h"
#include "ADSR.h"
#include "MoogFilter.h"
#include "Waveform.h"

// Oscillateurs de base
#include "PolyBLEP_Saw.h"
#include "PolyBLEP_Square.h"
#include "PolyBLEP_Triangle.h"
#include "NoiseOscillator.h"

// Nouveaux oscillateurs
#include "PolyBLEP_DoubleSaw.h"
#include "PolyBLEP_FilteredNoise.h"
#include "PolyBLEP_Pulse.h"
#include "PolyBLEP_RampDown.h"
#include "PolyBLEP_SubOsc.h"

#include "LFO.h"

struct SynthState {
    // Fréquence d'échantillonnage
    float sampleRate;

    // Groupe courant (0 : Oscillateurs, 1 : Amplitude ADSR, 2 : Filtre ADSR, 3 : LFO Depth ADSR)
    int oscControlGroup = 0;

    // Oscillateurs
    PolyBLEP_Saw* osc = nullptr;
    PolyBLEP_Square* oscSquare = nullptr;
    PolyBLEP_Triangle* oscTriangle = nullptr;
    NoiseOscillator* noiseOsc = nullptr;

    // Nouveaux oscillateurs
    PolyBLEP_DoubleSaw* doubleSawOsc = nullptr;
    PolyBLEP_FilteredNoise* filteredNoiseOsc = nullptr;
    PolyBLEP_Pulse* pulseOsc = nullptr;
    PolyBLEP_RampDown* rampDownOsc = nullptr;
    PolyBLEP_SubOsc* subOsc = nullptr;

    // Mix des oscillateurs
    float mixSaw = 1.0f;
    float mixSquare = 0.0f;
    float mixTriangle = 0.0f;
    float mixNoise = 0.0f;
    float mixDoubleSaw = 0.0f;
    float mixFilteredNoise = 0.0f;
    float mixPulse = 0.0f;
    float mixRampDown = 0.0f;
    float mixSub = 0.0f;
    float mixNoiseVolume = 0.0f;

    // Voix
    const int maxVoices = 6;
    std::vector<Voice> voices;

    Waveform* waveform = nullptr;

    // Contrôles
    float volume = 0.8f;
    int octave = 4;
    bool noteOn = false;

    // === ADSR amplitude ===
    ADSR env;
    float attackTime = 0.05f;
    float decayTime = 0.9f;
    float sustainLevel = 0.9f;
    float releaseTime = 0.9f;

    // === ADSR filtre ===
    float filterAttackTime = 0.01f;
    float filterDecayTime = 0.1f;
    float filterSustainLevel = 1.0f;
    float filterReleaseTime = 0.5f;

    // === ADSR LFO Depth ===
    float lfoEnvAttackTime = 0.01f;
    float lfoEnvDecayTime = 0.1f;
    float lfoEnvSustainLevel = 1.0f;
    float lfoEnvReleaseTime = 0.5f;

    // === Filtre ===
    MoogFilter moogFilter;
    float currentCutoff = 1000.0f;
    float currentResonance = 0.5f;
    float userResonance = 0.0f;

    // === LFO ===
    float lfoDepth = 10.0f;

    // LFO - Système hiérarchique
    float lfoGlobalDepth = 0.0f;        // CC3 - Profondeur globale (0-1)
    float lfoFrequency = 5.0f;          // CC4 - Fréquence LFO
    
    // Profondeurs relatives (0-1, multipliées par lfoGlobalDepth)
    float lfoDepthCutoff = 0.0f;        // CC9 - Profondeur cutoff relative
    float lfoDepthPitch = 0.0f;         // CC7 - Profondeur pitch relative  
    float pulseWidthDepth = 0.0f;       // CC8 - Profondeur PWM relative

    // Paramètres de base pour les calculs relatifs
    float baseCutoff = 1000.0f;

    LFO lfo;
    bool lfoEnabled = false;

    // Visualisation (waveform rendering)
    static constexpr int WaveformBufferSize = 2048;
    float renderBuffer[WaveformBufferSize] = {0.0f};
    int renderBufferIndex = 0;

    float renderBuffers[6][WaveformBufferSize] = {{0.0f}};
    int renderBufferIndices[6] = {0};

    // === Constructeur ===
    SynthState(float sampleRate)
        : sampleRate(sampleRate),
          env(sampleRate),
          moogFilter(sampleRate),
          lfo(sampleRate)
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
