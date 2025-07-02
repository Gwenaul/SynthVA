#include "Voice.h"

Voice::Voice(float sampleRate)
    : osc(sampleRate), oscSquare(sampleRate), oscTriangle(sampleRate), 
      noiseOsc(sampleRate), oscPulse(sampleRate), oscRampDown(sampleRate),
      oscSubOsc(sampleRate), oscDoubleSaw(sampleRate), oscFilteredNoise(sampleRate),
      env(sampleRate), filterEnv(sampleRate), lfoDepthEnv(sampleRate),
      moogFilter(sampleRate), active(false), freq(0.0f) {}

void Voice::setFrequency(float frequency) {
    freq = frequency;
    osc.setFrequency(frequency);
    oscSquare.setFrequency(frequency);
    oscTriangle.setFrequency(frequency);
    oscPulse.setFrequency(frequency);
    oscRampDown.setFrequency(frequency);
    oscSubOsc.setFrequency(frequency / 2.0f); // sub
    oscDoubleSaw.setFrequency(frequency);
}

void Voice::noteOn() {
    osc.resetPhase();
    oscSquare.resetPhase();
    oscTriangle.resetPhase();
    oscPulse.resetPhase();
    oscRampDown.resetPhase();
    oscSubOsc.resetPhase();
    oscDoubleSaw.resetPhase();
    oscFilteredNoise.resetPhase();
    
    env.noteOn();
    filterEnv.noteOn();
    lfoDepthEnv.noteOn();
    active = true;
}

void Voice::noteOff() {
    env.noteOff();
    filterEnv.noteOff();
    lfoDepthEnv.noteOff();
}

bool Voice::isActive() const {
    // return active && (env.isActive() || filterEnv.isActive() || lfoDepthEnv.isActive());
    return active && env.isActive();
}

void Voice::setPulseWidth(float width) {
    oscPulse.setPulseWidth(width);
}

void Voice::setNoiseType(NoiseOscillator::NoiseType type) {
    noiseOsc.setNoiseType(type);
}

