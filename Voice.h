#ifndef VOICE_H
#define VOICE_H

#include "PolyBLEP_Saw.h"
#include "PolyBLEP_Square.h"
#include "PolyBLEP_Triangle.h"
#include "NoiseOscillator.h"
#include "ADSR.h"
#include "MoogFilter.h"

class Voice {
public:
    Voice(float sampleRate);

    float process(float mixSaw, float mixSquare, float mixTriangle, float mixNoise);
    void setFrequency(float frequency);
    void noteOn();
    void noteOff();
    bool isActive() const;

    // Ajout du filtre Moog par voix
    MoogFilter moogFilter;
    
// private:
    PolyBLEP_Saw osc;
    PolyBLEP_Square oscSquare;
    PolyBLEP_Triangle oscTriangle;
    NoiseOscillator noiseOsc;
    ADSR env;

    bool active;
    float freq;

    void setNoiseType(NoiseOscillator::NoiseType type);
};

#endif // VOICE_H
