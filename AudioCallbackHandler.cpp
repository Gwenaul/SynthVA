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

        for (auto& voice : state->voices) {
            if (voice.active) {
                // Seulement traiter cette voix si elle est active
                if (voice.env.isActive()) {
                    float baseFreq = voice.freq;
                    float modulatedFreq = baseFreq;
                    
                    // Appliquer la modulation LFO à la fréquence
                    if (state->lfoEnabled) {
                        modulatedFreq += state->lfoDepth * lfoValue;
                    }

                    // Appliquer la fréquence modulée à chaque oscillateur
                    voice.osc.setFrequency(modulatedFreq);
                    voice.oscSquare.setFrequency(modulatedFreq);
                    voice.oscTriangle.setFrequency(modulatedFreq);

                    // Générer le mix d'oscillateurs
                    float oscMix = 0.0f;
                    oscMix += state->mixSaw     * voice.osc.process();
                    oscMix += state->mixSquare  * voice.oscSquare.process();
                    oscMix += state->mixTriangle* voice.oscTriangle.process();
                    oscMix += state->mixNoise   * voice.noiseOsc.process();

                    // Appliquer l'enveloppe ADSR au signal
                    float envValue = voice.env.process();
                    
                    // Éviter les artefacts si l'enveloppe est très faible
                    if (envValue > 0.0001f) {
                        // Appliquer l'enveloppe au signal
                        float enveloped = oscMix * envValue;
                        
                        // Filtrer le signal
                        float filtered = state->moogFilter.process(enveloped);
                        
                        // Ajouter au mix final
                        mix += filtered;
                    } else if (!voice.env.isActive()) {
                        // Si l'enveloppe est terminée, marquer la voix comme inactive
                        voice.active = false;
                    }
                } else {
                    // Si l'enveloppe n'est plus active, marquer la voix comme inactive
                    voice.active = false;
                }
            }
        }

        // Appliquer le volume global
        mix *= volume;
        
        // Limiter le signal pour éviter l'écrêtage
        if (mix > 1.0f) mix = 1.0f;
        if (mix < -1.0f) mix = -1.0f;

        *out++ = mix;  // Canal gauche
        *out++ = mix;  // Canal droit
    }

    return paContinue;
}