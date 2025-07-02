#include "AudioCallbackHandler.h"
#include <iostream>
#include <portaudio.h>
#include "LFO.h"
#include "Clamp.h"

float AudioCallbackHandler::calculateMusicalCutoffModulation(float baseCutoff, float globalDepth, 
                                                             float cutoffDepth, float lfoValue) {
    if (globalDepth <= 0.0f || cutoffDepth <= 0.0f) {
        return baseCutoff;
    }

    float effectiveDepth = globalDepth * cutoffDepth;
    float baseOctave = log2f(baseCutoff / 55.0f);
    float modulationOctaves = effectiveDepth * lfoValue * 1.0f;
    float newOctave = baseOctave + modulationOctaves;
    float modulatedCutoff = 55.0f * powf(2.0f, newOctave);

    return clamp(modulatedCutoff, 30.0f, 18000.0f);
}

int AudioCallbackHandler::audioCallback(const void *inputBuffer, void *outputBuffer,
                                        unsigned long framesPerBuffer,
                                        const PaStreamCallbackTimeInfo *timeInfo,
                                        PaStreamCallbackFlags statusFlags,
                                        void *userData) {
    SynthState* state = (SynthState*)userData;
    float* out = (float*)outputBuffer;

    if (!state || !state->osc) {
        std::cerr << "Error: Oscillator or envelope not initialized!" << std::endl;
        return paAbort;
    }

    float volume = state->volume;

    static int cleanupCounter = 0;
    if (++cleanupCounter >= 4410) { // nettoyage périodique
        state->cleanupVoices();
        cleanupCounter = 0;
    }

    for (unsigned long i = 0; i < framesPerBuffer; ++i) {
    float lfoValue = state->lfoEnabled ? state->lfo.process() : 0.0f;

    float leftMix = 0.0f;
    float rightMix = 0.0f;

    int voiceIndex = 0;

    for (auto& voice : state->voices) {
        if (voice.active && voice.env.isActive()) {
            float baseFreq = voice.freq;
            float modulatedFreq = baseFreq;
            float envValue = voice.env.process();
            float filterEnvValue = voice.filterEnv.process();
            float lfoEnvDepth = voice.lfoDepthEnv.process();

            // --- Pitch modulation ---
            if (state->lfoEnabled && state->lfoDepthPitch > 0.0f) {
                float pitchModSemitones = state->lfoGlobalDepth * lfoEnvDepth * state->lfoDepthPitch * lfoValue * 12.0f;
                modulatedFreq = baseFreq * powf(2.0f, pitchModSemitones / 12.0f);
            }

            voice.osc.setFrequency(modulatedFreq);
            voice.oscSquare.setFrequency(modulatedFreq);
            voice.oscTriangle.setFrequency(modulatedFreq);
            voice.oscPulse.setFrequency(modulatedFreq);
            voice.oscRampDown.setFrequency(modulatedFreq);
            voice.oscSubOsc.setFrequency(modulatedFreq);
            voice.oscDoubleSaw.setFrequency(modulatedFreq);

            // --- PWM modulation ---
            if (state->lfoEnabled && state->pulseWidthDepth > 0.0f) {
                float pwm = 0.5f + state->lfoGlobalDepth * lfoEnvDepth * state->pulseWidthDepth * lfoValue * 0.4f;
                pwm = clamp(pwm, 0.05f, 0.95f);
                voice.setPulseWidth(pwm);
            }

            // Combine LFO et Envelope pour moduler le cutoff
            float lfoModulatedCutoff = calculateMusicalCutoffModulation(
                state->baseCutoff,
                state->lfoGlobalDepth * lfoEnvDepth, // modulation dynamique
                state->lfoDepthCutoff,
                lfoValue
            );

            // Ajoute la modulation de l'enveloppe filtre (linéaire ici)
            // float finalCutoff = lfoModulatedCutoff * (0.5f + filterEnvValue); // ou autre courbe
            // float finalCutoff = lfoModulatedCutoff + (lfoModulatedCutoff * filterEnvValue);
            // float finalCutoff = lfoModulatedCutoff + (lfoModulatedCutoff * filterEnvValue * 2.0f);
            float envModulation = filterEnvValue * 2.0f; // Facteur de modulation
            float finalCutoff = lfoModulatedCutoff * (1.0f + envModulation);

            finalCutoff = clamp(finalCutoff, 30.0f, 18000.0f);
            voice.moogFilter.setCutoff(finalCutoff);
            voice.moogFilter.setResonance(state->currentResonance);

            float oscMix = 0.0f;
            oscMix += state->mixSaw      * voice.osc.process();
            oscMix += state->mixSquare   * voice.oscSquare.process();
            oscMix += state->mixTriangle * voice.oscTriangle.process();
            oscMix += state->mixNoise    * voice.noiseOsc.process();
            oscMix += state->mixPulse    * voice.oscPulse.process();
            oscMix += state->mixRampDown * voice.oscRampDown.process();
            oscMix += state->mixDoubleSaw * voice.oscDoubleSaw.process();
            oscMix += state->mixSub       * voice.oscSubOsc.process();
            oscMix += state->mixFilteredNoise * voice.oscFilteredNoise.process();

            if (envValue > 0.0001f) {
                float enveloped = oscMix * envValue;
                float filtered = voice.moogFilter.process(enveloped);

                float pan = voice.pan; // [0.0 - 1.0]
                leftMix  += filtered * (1.0f - pan) * 0.05f;
                rightMix += filtered * pan         * 0.05f;

                state->renderBuffers[voiceIndex][state->renderBufferIndices[voiceIndex]] = filtered;
                state->renderBufferIndices[voiceIndex] = (state->renderBufferIndices[voiceIndex] + 1) % SynthState::WaveformBufferSize;
            } else if (!voice.env.isActive()) {
                voice.active = false;
            }
        } else {
            voice.active = false;
        }

        voiceIndex++;
    }

    leftMix  *= volume;
    rightMix *= volume;

    state->renderBuffer[state->renderBufferIndex] = (leftMix + rightMix) * 0.5f;
    state->renderBufferIndex = (state->renderBufferIndex + 1) % SynthState::WaveformBufferSize;

    *out++ = leftMix;
    *out++ = rightMix;
}

    return paContinue;
}