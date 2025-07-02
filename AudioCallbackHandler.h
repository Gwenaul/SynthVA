#ifndef AUDIO_CALLBACK_HANDLER_H
#define AUDIO_CALLBACK_HANDLER_H

#include "SynthState.h"
#include <portaudio.h>

class AudioCallbackHandler {
public:
    static int audioCallback(const void *inputBuffer, void *outputBuffer,
                             unsigned long framesPerBuffer,
                             const PaStreamCallbackTimeInfo *timeInfo,
                             PaStreamCallbackFlags statusFlags,
                             void *userData);

private:
    // Fonction pour calculer la modulation cutoff de manière musicale
    static float calculateMusicalCutoffModulation(float baseCutoff, float globalDepth, 
                                        float cutoffDepth, float lfoValue);
    // Normalise la somme des mixages d'oscillateurs pour éviter la saturation
    static float normalizeMix(float saw, float square, float triangle, float noise);
};

#endif // AUDIO_CALLBACK_HANDLER_H
