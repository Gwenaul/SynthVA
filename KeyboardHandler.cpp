// KeyboardHandler.cpp
#include "KeyboardHandler.h"
#include "SynthState.h"

void handleKeyboard(SynthState& state, SDL_Event& event, bool& quit) {
    if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
        quit = true;
    } 
    else if (event.type == SDL_KEYDOWN) {
        SDL_Keycode key = event.key.keysym.sym;
        if (key == SDLK_a && (event.key.keysym.mod & KMOD_SHIFT)) {
            state.env.setAttack(state.env.getAttack() + 0.01f);
        }
        else if (key == SDLK_i && (event.key.keysym.mod & KMOD_SHIFT)) {
            state.env.setDecay(state.env.getDecay() + 0.01f);
        }
        else if (key == SDLK_o && (event.key.keysym.mod & KMOD_SHIFT)) {
            state.env.setSustain(state.env.getSustain() + 0.01f);
        }
        else if (key == SDLK_r && (event.key.keysym.mod & KMOD_SHIFT)) {
            state.env.setRelease(state.env.getRelease() + 0.01f);
        }
        else if (key == SDLK_w) {
            state.octave = std::min(state.octave + 1, 8);
            std::cout << "Octave: " << state.octave << std::endl;
        } 
        else if (key == SDLK_x) {
            state.octave = std::max(state.octave - 1, 1);
            std::cout << "Octave: " << state.octave << std::endl;
        } 
        else if (key == SDLK_RETURN) {
            for (auto& voice : state.voices) {
                voice.env.noteOff();
                voice.active = false;
            }
            std::cout << "All notes OFF" << std::endl;
        }
        else if (key == SDLK_c) {
            state.volume = std::max(0.0f, state.volume - 0.05f);
            std::cout << "Volume: " << (state.volume) * 100 << "%" << std::endl;
        } 
        else if (key == SDLK_v) {
            state.volume = std::min(1.0f, state.volume + 0.05f);
            std::cout << "Volume: " << (state.volume) * 100 << "%" << std::endl;
        }
        else if (key == SDLK_n) { 
            float newCutoff = std::max(20.0f, state.moogFilter.getCutoff() - 200.0f);
            state.moogFilter.setCutoff(newCutoff);
            std::cout << "Cutoff: " << newCutoff << " Hz" << std::endl;
        }
        else if (key == SDLK_m) { 
            float newCutoff = std::min(20000.0f, state.moogFilter.getCutoff() + 200.0f);
            state.moogFilter.setCutoff(newCutoff);
            std::cout << "Cutoff: " << newCutoff << " Hz" << std::endl;
        }
        else if (key == SDLK_r) { 
            float newRes = std::max(0.0f, state.moogFilter.getResonance() - 0.1f);
            state.moogFilter.setResonance(newRes);
            std::cout << "Resonance: " << newRes << std::endl;
        }
        else if (key == SDLK_i) { 
            float newRes = std::min(4.0f, state.moogFilter.getResonance() + 0.1f);
            state.moogFilter.setResonance(newRes);
            std::cout << "Resonance: " << newRes << std::endl;
        }

        else if (key == SDLK_1) { // Saw up
            state.mixSaw = std::min(1.0f, state.mixSaw + 0.1f);
            std::cout << "Saw mix: " << state.mixSaw << std::endl;
        }
        else if (key == SDLK_2) { // Saw down
            state.mixSaw = std::max(0.0f, state.mixSaw - 0.1f);
            std::cout << "Saw mix: " << state.mixSaw << std::endl;
        }
        else if (key == SDLK_3) { // Square up
            state.mixSquare = std::min(1.0f, state.mixSquare + 0.1f);
            std::cout << "Square mix: " << state.mixSquare << std::endl;
        }
        else if (key == SDLK_4) { // Square down
            state.mixSquare = std::max(0.0f, state.mixSquare - 0.1f);
            std::cout << "Square mix: " << state.mixSquare << std::endl;
        }
        else if (key == SDLK_5) { // Triangle up
            state.mixTriangle = std::min(1.0f, state.mixTriangle + 0.1f);
            std::cout << "Triangle mix: " << state.mixTriangle << std::endl;
        }
        else if (key == SDLK_6) { // Triangle down
            state.mixTriangle = std::max(0.0f, state.mixTriangle - 0.1f);
            std::cout << "Triangle mix: " << state.mixTriangle << std::endl;
        }
        else if (key == SDLK_7) { // Noise up
            state.mixNoise = std::min(1.0f, state.mixNoise + 0.1f);
            std::cout << "Noise mix: " << state.mixNoise << std::endl;
        }
        else if (key == SDLK_8) { // Noise down
            state.mixNoise = std::max(0.0f, state.mixNoise - 0.1f);
            std::cout << "Noise mix: " << state.mixNoise << std::endl;
        }

        else if (key == SDLK_9) {
            state.lfoEnabled = !state.lfoEnabled;
            std::cout << "LFO " << (state.lfoEnabled ? "ON" : "OFF") << std::endl;
        }

        else if (key == SDLK_o) {
            state.lfoFrequency = std::min(20.0f, state.lfoFrequency + 0.5f);
            state.lfo.setFrequency(state.lfoFrequency);
            std::cout << "LFO freq: " << state.lfoFrequency << " Hz" << std::endl;
        }
        else if (key == SDLK_p) {
            state.lfoFrequency = std::max(0.1f, state.lfoFrequency - 0.5f);
            state.lfo.setFrequency(state.lfoFrequency);
            std::cout << "LFO freq: " << state.lfoFrequency << " Hz" << std::endl;
        }

        else if (key == SDLK_EQUALS) {
            state.lfoDepth = std::max(0.0f, state.lfoDepth - 1.0f);
            std::cout << "LFO depth: " << state.lfoDepth << " Hz" << std::endl;
        }
        else if (key == SDLK_MINUS) {
            state.lfoDepth = std::min(100.0f, state.lfoDepth + 1.0f);
            std::cout << "LFO depth: " << state.lfoDepth << " Hz" << std::endl;
        }

        else {
            float freq = keyToFreq(key, state.octave);
            if (freq > 0.0f) {
                bool alreadyPlaying = false;
                for (auto& voice : state.voices) {
                    if (voice.freq == freq && voice.active) {
                        alreadyPlaying = true;
                        break;
                    }
                }
                if (!alreadyPlaying) {
                    Voice* selectedVoice = nullptr;
                    for (auto& voice : state.voices) {
                        if (!voice.active) {
                            selectedVoice = &voice;
                            break;
                        }
                    }
                    if (!selectedVoice) {
                        selectedVoice = &state.voices[0];
                    }
                    selectedVoice->osc.setFrequency(freq);
                    selectedVoice->oscSquare.setFrequency(freq);
                    selectedVoice->oscTriangle.setFrequency(freq);
                    selectedVoice->env.noteOn();
                    selectedVoice->active = true;
                    selectedVoice->freq = freq;
                    std::cout << "Note ON: " << freq << " Hz (octave " << state.octave << ")" << std::endl;
                }
            }
        }
    }
    else if (event.type == SDL_KEYUP) {
        SDL_Keycode key = event.key.keysym.sym;
        float freq = keyToFreq(key, state.octave);
        if (freq > 0.0f) {
            for (auto& voice : state.voices) {
                if (voice.freq == freq && voice.active) {
                    voice.env.noteOff();
                    voice.active = false;
                    std::cout << "Note released: " << freq << " Hz" << std::endl;
                    break;
                }
            }
        }
    }
}
