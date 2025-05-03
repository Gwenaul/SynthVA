// KeyMapping.h
#ifndef KEYMAPPING_H
#define KEYMAPPING_H

#include <SDL2/SDL.h> // Pour SDL_Keycode
#include <cmath>      // Pour pow()

float keyToFreq(SDL_Keycode key, int octave);

#endif // KEYMAPPING_H
