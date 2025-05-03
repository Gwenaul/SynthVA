#include "Voice.h"

Voice::Voice(float sampleRate)
    : osc(sampleRate), oscSquare(sampleRate), oscTriangle(sampleRate), noiseOsc(sampleRate), env(sampleRate),
      active(false), freq(0.0f) {}

void Voice::setFrequency(float frequency) {
    freq = frequency;
}

void Voice::noteOn() {
    env.noteOn();
    active = true;
}

void Voice::noteOff() {
    env.noteOff();
}

    bool Voice::isActive() const {
    return active && env.isActive();
}

float Voice::process(float mixSaw, float mixSquare, float mixTriangle, float mixNoise) {
    if (!active) return 0.0f;

    float s = osc.process();
    float sq = oscSquare.process();
    float tri = oscTriangle.process();
    float noise = noiseOsc.process();

    float mixed = mixSaw * s + mixSquare * sq + mixTriangle * tri + mixNoise * noise;
    float amp = env.process();
    float output = mixed * amp;

    return output;
}

