// KeyboardHandler.h
#ifndef KEYBOARD_HANDLER_H
#define KEYBOARD_HANDLER_H

#include <SDL2/SDL.h>
#include <iostream>
#include <algorithm>
#include "SynthState.h"
#include "Voice.h"
#include "KeyMapping.h" // pour keyToFreq()

void handleKeyboard(SynthState& state, SDL_Event& event, bool& quit);

#endif // KEYBOARD_HANDLER_H
