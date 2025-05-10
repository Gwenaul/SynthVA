#include "AudioCallbackHandler.h"
#include <iostream>
#include <portaudio.h>
#include "LFO.h"

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

    // Nettoyer périodiquement les voix inactives
    static int cleanupCounter = 0;
    if (++cleanupCounter >= 4410) { // Par exemple, toutes les 0.1 secondes à 44.1 kHz
        state->cleanupVoices();
        cleanupCounter = 0;
    }

    for (unsigned long i = 0; i < framesPerBuffer; ++i) {
        float mix = 0.0f;

        // Calculer une seule fois la valeur LFO par échantillon
        float lfoValue = state->lfoEnabled ? state->lfo.process() : 0.0f;
        //pour compter les voix actives dans la boucle suivante
        int activeVoices = 0;

        float filtered = 0.0f;

        int voiceIndex = 0;
        
        for (auto& voice : state->voices) {
            if (voice.active) {
                if (voice.env.isActive()) {
                    float baseFreq = voice.freq;
                    float modulatedFreq = baseFreq;

                    if (state->lfoEnabled) {
                        modulatedFreq += state->lfoDepth * lfoValue;
                    }

                    voice.osc.setFrequency(modulatedFreq);
                    voice.oscSquare.setFrequency(modulatedFreq);
                    voice.oscTriangle.setFrequency(modulatedFreq);

                    voice.moogFilter.setCutoff(state->currentCutoff);
                    voice.moogFilter.setResonance(state->currentResonance);

                    float oscMix = 0.0f;
                    oscMix += state->mixSaw      * voice.osc.process();
                    oscMix += state->mixSquare   * voice.oscSquare.process();
                    oscMix += state->mixTriangle * voice.oscTriangle.process();
                    oscMix += state->mixNoise    * voice.noiseOsc.process();

                    float envValue = voice.env.process();

                    if (envValue > 0.0001f) {
                        float enveloped = oscMix * envValue;
                        float filtered = voice.moogFilter.process(enveloped);

                        mix += filtered * 0.05f;

                        // Écriture correcte dans le buffer circulaire pour CETTE voix
                        state->renderBuffers[voiceIndex][state->renderBufferIndices[voiceIndex]] = filtered;
                        state->renderBufferIndices[voiceIndex] = (state->renderBufferIndices[voiceIndex] + 1) % SynthState::WaveformBufferSize;
                    } else if (!voice.env.isActive()) {
                        voice.active = false;
                    }
                } else {
                    voice.active = false;
                }
            }

            voiceIndex++; // Avancer à la prochaine voix
        }

        // Appliquer le volume global
        mix *= volume;

        // WaveformRenderer
        state->renderBuffer[state->renderBufferIndex] = mix;
        state->renderBufferIndex = (state->renderBufferIndex + 1) % SynthState::WaveformBufferSize;

        *out++ = mix;  // Canal gauche
        *out++ = mix;  // Canal droit
    }

    return paContinue;
}