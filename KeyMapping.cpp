// KeyMapping.cpp
#include "KeyMapping.h"

float keyToFreq(SDL_Keycode key, int octave) {
    float baseFreqs[] = {
        261.63f,  // C
        277.18f,  // C#
        293.66f,  // D
        311.13f,  // D#
        329.63f,  // E
        349.23f,  // F
        369.99f,  // F#
        392.00f,  // G
        415.30f,  // G#
        440.00f,  // A
        466.16f,  // A#
        493.88f   // B
    };

    int note = -1;
    switch (key) {
        case SDLK_q: note = 0; break;  // C
        case SDLK_z: note = 1; break;  // C#
        case SDLK_s: note = 2; break;  // D
        case SDLK_e: note = 3; break;  // D#
        case SDLK_d: note = 4; break;  // E
        case SDLK_f: note = 5; break;  // F
        case SDLK_t: note = 6; break;  // F#
        case SDLK_g: note = 7; break;  // G
        case SDLK_y: note = 8; break;  // G#
        case SDLK_h: note = 9; break;  // A
        case SDLK_u: note = 10; break; // A#
        case SDLK_j: note = 11; break; // B
        case SDLK_k: note = 0; octave += 1; break; // C next octave
        default: return 0.0f;
    }

    return note >= 0 ? baseFreqs[note] * pow(2, octave - 4) : 0.0f;
}
