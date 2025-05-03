#ifndef MIDICALLBACK_H
#define MIDICALLBACK_H

#include <vector>
#include "SynthState.h"

// Déclaration de la fonction callback MIDI
void midiCallback(double deltatime, std::vector<unsigned char> *message, void *userData);

#endif // MIDICALLBACK_H
