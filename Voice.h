#ifndef VOICE_H
#define VOICE_H

#include "PolyBLEP_Saw.h"
#include "PolyBLEP_Square.h"
#include "PolyBLEP_Triangle.h"
#include "PolyBLEP_Pulse.h"
#include "PolyBLEP_RampDown.h"
#include "PolyBLEP_SubOsc.h"
#include "PolyBLEP_DoubleSaw.h"
#include "PolyBLEP_FilteredNoise.h"
#include "NoiseOscillator.h"
#include "ADSR.h"
#include "MoogFilter.h"

class Voice {
public:
    Voice(float sampleRate);

    void setFrequency(float frequency);
    void noteOn();
    void noteOff();
    bool isActive() const;

    // // Contrôles spécifiques aux nouvelles formes d'ondes
    void setPulseWidth(float width);
    void setNoiseType(NoiseOscillator::NoiseType type);

    float pan = 0.5f; // 0.0 = gauche, 1.0 = droite

    // Filtre Moog par voix
    MoogFilter moogFilter;

    // Enveloppes
    ADSR env;            // Amplitude
    ADSR filterEnv;      // Cutoff
    ADSR lfoDepthEnv;    // LFO Depth

// private:
    bool active = false;
    float freq = 440.0f;

    // Oscillateurs
    PolyBLEP_Saw osc;
    PolyBLEP_Square oscSquare;
    PolyBLEP_Triangle oscTriangle;
    NoiseOscillator noiseOsc;
    PolyBLEP_Pulse oscPulse;
    PolyBLEP_RampDown oscRampDown;
    PolyBLEP_SubOsc oscSubOsc;
    PolyBLEP_DoubleSaw oscDoubleSaw;
    PolyBLEP_FilteredNoise oscFilteredNoise;
};

#endif // VOICE_H
