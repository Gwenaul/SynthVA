#ifndef MIDICALLBACK_H
#define MIDICALLBACK_H

#include <vector>
#include "SynthState.h"
#include "MidiLooper.h"

// Déclaration de la fonction callback MIDI
void midiCallback(double deltatime, std::vector<unsigned char> *message, void *userData);
void setupMidiLooper(SynthState* state);

#endif // MIDICALLBACK_H
