#ifndef AUDIO_CALLBACK_HANDLER_H
#define AUDIO_CALLBACK_HANDLER_H

#include "SynthState.h"
#include "MoogFilter.h"
#include "Voice.h"
#include <portaudio.h>

class AudioCallbackHandler {
public:
    static int audioCallback(const void *inputBuffer, void *outputBuffer,
                             unsigned long framesPerBuffer,
                             const PaStreamCallbackTimeInfo *timeInfo,
                             PaStreamCallbackFlags statusFlags,
                             void *userData);
};

#endif // AUDIO_CALLBACK_HANDLER_H
