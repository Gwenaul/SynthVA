#include "MidiHandler.h"
#include <iostream>
#include <vector>
#include <cmath>
#include "Clamp.h"

// Déclaration externe au lieu de définition
extern MidiLooper midiLooper;

float getSymmetricalPan(int index) {
    int half = index / 2;
    float offset = 0.1f + 0.1f * half; // espacement par cran : 0.1 (10%)
    if (index == 0) return 0.5f; // premier au centre
    return (index % 2 == 1) ? 0.5f + offset : 0.5f - offset;
}

float computeEffectiveResonance(float userRes, float cutoff) {
    float normCutoff = clamp((cutoff - 100.0f) / (20000.0f - 100.0f), 0.0f, 1.0f);
    float adjustedRes = userRes * (1.2f - normCutoff);
    return clamp(adjustedRes, 0.0f, 4.0f);
}

void midiCallback(double deltatime, std::vector<unsigned char> *message, void *userData) {
    SynthState* state = static_cast<SynthState*>(userData);

    if (message->size() >= 3) {
        unsigned char status = message->at(0);

        unsigned char messageType = status & 0xF0;
        unsigned char channel = status & 0x0F;

        if (channel != 0) return; // Canal MIDI 1 (canal 0 dans le byte MIDI)

        unsigned char control = message->at(1);
        unsigned char value = message->at(2);

        // Vérifier que c'est un Control Change (status 0xB0 à 0xBF)
        if ((messageType) == 0xB0) {

            // Pour capter les commandes transport
            midiLooper.processMidiMessage(*message);

            if (control == 115 && value > 0) {
                // Groupe précédent (0 → 3 → 2 → 1 → 0)
                state->oscControlGroup = (state->oscControlGroup + 3) % 4; // +3 équivaut à -1 modulo 4
                std::cout << "[MIDI] Switched to oscillator group: " << state->oscControlGroup << std::endl;
                return;
            }

            // Switch entre groupes d’oscillateurs avec CC 115/116
            if (control == 116 && value > 0) {
                // Groupe suivant (0 → 1 → 2 → 3 → 0)
                state->oscControlGroup = (state->oscControlGroup + 1) % 4;
                std::cout << "[MIDI] Switched to oscillator group: " << state->oscControlGroup << std::endl;
                return;
            }

            if (control >= 7 && control <= 14) {
                if (state->oscControlGroup == 0) {
                    switch (control) {
                        case 7: state->mixSaw = clamp(value / 127.0f, 0.0f, 1.0f); break;
                        case 8: state->mixDoubleSaw = clamp(value / 127.0f, 0.0f, 1.0f); break;
                        case 9: state->mixTriangle = clamp(value / 127.0f, 0.0f, 1.0f); break;
                        case 10: state->mixSquare = clamp(value / 127.0f, 0.0f, 1.0f); break;
                        case 11: state->mixPulse = clamp(value / 127.0f, 0.0f, 1.0f); break;
                        case 12: state->mixRampDown = clamp(value / 127.0f, 0.0f, 1.0f); break;
                        case 13: state->mixSub = clamp(value / 127.0f, 0.0f, 1.0f); break;
                        case 14: state->mixNoise = clamp(value / 127.0f, 0.0f, 1.0f); break;
                    }
                } else if (state->oscControlGroup == 1) {
                    switch (control) {
                          case 7: { // LFO → pitch depth (relatif)
                            state->lfoDepthPitch = clamp(value / 127.0f, 0.0f, 1.0f);
                            break;
                        }
                        case 8: { // LFO → pulse width depth (relatif)
                            state->pulseWidthDepth = clamp(value / 127.0f, 0.0f, 1.0f);
                            break;
                        }
                        case 9: { // LFO → cutoff depth (relatif)
                            state->lfoDepthCutoff = clamp(value / 127.0f, 0.0f, 1.0f);
                            break;
                        }
                        case 11: {
                            float attackTime = 0.001f + (value / 127.0f) * 1.999f;
                            state->attackTime = attackTime;
                            for (auto& voice : state->voices)
                                voice.env.setAttack(attackTime);
                            break;
                        }
                        case 12: {
                            float decayTime = 0.001f + (value / 127.0f) * 2.999f;
                            state->decayTime = decayTime;
                            for (auto& voice : state->voices)
                                voice.env.setDecay(decayTime);
                            break;
                        }
                        case 13: {
                            float sustainLevel = value / 127.0f;
                            state->sustainLevel = sustainLevel;
                            for (auto& voice : state->voices)
                                voice.env.setSustain(sustainLevel);
                            break;
                        }
                        case 14: {
                            float releaseTime = 0.001f + (value / 127.0f) * 4.999f;
                            state->releaseTime = releaseTime;
                            for (auto& voice : state->voices)
                                voice.env.setRelease(releaseTime);
                            break;
                        }
                    }
                } else if (state->oscControlGroup == 2) {
                    // Contrôles pour l'enveloppe de filtre
                    switch (control) {
                        case 11: {
                            float attackTime = 0.001f + (value / 127.0f) * 1.999f;
                            state->filterAttackTime = attackTime;
                            for (auto& voice : state->voices)
                                voice.filterEnv.setAttack(attackTime);
                            break;
                        }
                        case 12: {
                            float decayTime = 0.001f + (value / 127.0f) * 2.999f;
                            state->filterDecayTime = decayTime;
                            for (auto& voice : state->voices)
                                voice.filterEnv.setDecay(decayTime);
                            break;
                        }
                        case 13: {
                            float sustainLevel = value / 127.0f;
                            state->filterSustainLevel = sustainLevel;
                            for (auto& voice : state->voices)
                                voice.filterEnv.setSustain(sustainLevel);
                            break;
                        }
                        case 14: {
                            float releaseTime = 0.001f + (value / 127.0f) * 4.999f;
                            state->filterReleaseTime = releaseTime;
                            for (auto& voice : state->voices)
                                voice.filterEnv.setRelease(releaseTime);
                            break;
                        }
                    }
                } else if (state->oscControlGroup == 3) {
                    // Contrôles pour l'enveloppe de profondeur du LFO (lfoDepthEnv)
                    switch (control) {
                        case 11: {
                            float attackTime = 0.001f + (value / 127.0f) * 1.999f;
                            state->lfoEnvAttackTime = attackTime;
                            for (auto& voice : state->voices)
                                voice.lfoDepthEnv.setAttack(attackTime);
                            break;
                        }
                        case 12: {
                            float decayTime = 0.001f + (value / 127.0f) * 2.999f;
                            state->lfoEnvDecayTime = decayTime;
                            for (auto& voice : state->voices)
                                voice.lfoDepthEnv.setDecay(decayTime);
                            break;
                        }
                        case 13: {
                            float sustainLevel = value / 127.0f;
                            state->lfoEnvSustainLevel = sustainLevel;
                            for (auto& voice : state->voices)
                                voice.lfoDepthEnv.setSustain(sustainLevel);
                            break;
                        }
                        case 14: {
                            float releaseTime = 0.001f + (value / 127.0f) * 4.999f;
                            state->lfoEnvReleaseTime = releaseTime;
                            for (auto& voice : state->voices)
                                voice.lfoDepthEnv.setRelease(releaseTime);
                            break;
                        }
                    }
                }
            // Mapping des contrôles selon le groupe actif
            } else if (control == 1) { // Cutoff principal
                float cutoff = 100.0f * std::pow(200.0f, value / 127.0f);
                state->currentCutoff = cutoff;
                state->baseCutoff = cutoff; // Mise à jour de la référence
                
                float adjustedRes = computeEffectiveResonance(state->userResonance, cutoff);
                state->currentResonance = adjustedRes;
                
                for (auto& voice : state->voices) {
                    voice.moogFilter.setCutoff(cutoff);
                    voice.moogFilter.setResonance(adjustedRes);
                }
            } else if (control == 2) {
                float rawRes = (value / 127.0f) * 4.0f;
                state->userResonance = rawRes;

                float adjustedRes = computeEffectiveResonance(rawRes, state->currentCutoff);
                state->currentResonance = adjustedRes;

                for (auto& voice : state->voices) {
                    voice.moogFilter.setResonance(adjustedRes);
                }
            } else if (control == 3) { // LFO Global Depth
                state->lfoEnabled = (value >= 4);
                state->lfoGlobalDepth = clamp(value / 127.0f, 0.0f, 1.0f);
            } else if (control == 4) { // LFO Frequency
                // Mapping logarithmique de [0..127] vers [0.01 .. 20 Hz]
                float minHz = 0.01f;
                float maxHz = 20.0f;
                float t = value / 127.0f;
                float freq = minHz * powf((maxHz / minHz), t); // interpolation log
                state->lfoFrequency = freq;
                state->lfo.setFrequency(freq);
            } else if (control == 6) { // Volume
                state->volume = clamp(value / 127.0f, 0.0f, 1.0f);
            } else if (control == 5) {
                // Fader type de bruit : 4 positions
                // 0-31 : PINK noise normal
                // 32-63 : WHITE noise normal
                // 64-95 : BROWN noise normal
                // 96-127 : filteredNoise activé (mixFilteredNoise = 1), mixNoise = 0
                for (auto& voice : state->voices) {
                    if (value < 32) {
                        voice.setNoiseType(NoiseOscillator::NoiseType::PINK);
                        state->mixFilteredNoise = 0.0f;
                        state->mixNoise = state->mixNoiseVolume; // activé selon volume noise
                    } else if (value < 64) {
                        voice.setNoiseType(NoiseOscillator::NoiseType::WHITE);
                        state->mixFilteredNoise = 0.0f;
                        state->mixNoise = state->mixNoiseVolume;
                    } else if (value < 96) {
                        voice.setNoiseType(NoiseOscillator::NoiseType::BROWN);
                        state->mixFilteredNoise = 0.0f;
                        state->mixNoise = state->mixNoiseVolume;
                    } else {
                        // Position 4 : filtered noise activé, noise normal off
                        state->mixFilteredNoise = state->mixNoiseVolume;
                        state->mixNoise = 0.0f;
                    }
                }
            }
        }

        if ((messageType) == 0x90 && value > 0) {  // Note On
            int note = control;
            float freq = 440.0f * pow(2.0f, (note - 69) / 12.0f);

            midiLooper.processMidiMessage(*message);

            bool found = false;
            for (auto& voice : state->voices) {
                if (voice.active && voice.freq == freq) {
                    voice.env.noteOn();
                    voice.filterEnv.noteOn();
                    voice.lfoDepthEnv.noteOn();
                    found = true;
                    break;
                }
            }

            if (!found) {
                if (state->voices.size() >= state->maxVoices) {
                    int oldestVoiceIndex = -1;
                    for (size_t i = 0; i < state->voices.size(); i++) {
                        if (!state->voices[i].isActive()) {
                            oldestVoiceIndex = i;
                            break;
                        }
                    }
                    if (oldestVoiceIndex == -1) oldestVoiceIndex = 0;

                    auto& reusedVoice = state->voices[oldestVoiceIndex];
                    // float pan = 0.2f + (oldestVoiceIndex / 5.0f) * 0.6f;
                    float pan = getSymmetricalPan(oldestVoiceIndex);
                    reusedVoice.pan = pan;
                    reusedVoice.freq = freq;
                    reusedVoice.active = true;

                    reusedVoice.env.setAttack(state->attackTime);
                    reusedVoice.env.setDecay(state->decayTime);
                    reusedVoice.env.setSustain(state->sustainLevel);
                    reusedVoice.env.setRelease(state->releaseTime);

                    reusedVoice.env.noteOn();
                    reusedVoice.filterEnv.noteOn();
                    reusedVoice.lfoDepthEnv.noteOn();
                } else {
                    Voice voice(state->sampleRate);
                    int newIndex = state->voices.size(); // Index futur
                    // float pan = 0.2f + (newIndex / 5.0f) * 0.6f;
                    float pan = getSymmetricalPan(newIndex);
                    voice.pan = pan;

                    voice.env.setAttack(state->attackTime);
                    voice.env.setDecay(state->decayTime);
                    voice.env.setSustain(state->sustainLevel);
                    voice.env.setRelease(state->releaseTime);

                    voice.filterEnv.setAttack(state->filterAttackTime);
                    voice.filterEnv.setDecay(state->filterDecayTime);
                    voice.filterEnv.setSustain(state->filterSustainLevel);
                    voice.filterEnv.setRelease(state->filterReleaseTime);

                    voice.lfoDepthEnv.setAttack(state->lfoEnvAttackTime);
                    voice.lfoDepthEnv.setDecay(state->lfoEnvDecayTime);
                    voice.lfoDepthEnv.setSustain(state->lfoEnvSustainLevel);
                    voice.lfoDepthEnv.setRelease(state->lfoEnvReleaseTime);

                    voice.freq = freq;
                    voice.active = true;
                    voice.env.noteOn();
                    voice.filterEnv.noteOn();
                    voice.lfoDepthEnv.noteOn();

                    state->voices.push_back(voice);
                }
            }
            std::cout << "Note On: " << note << " -> " << freq << " Hz" << std::endl;
        }

        else if ((messageType) == 0x80 || ((status & 0xF0) == 0x90 && value == 0)) {  // Note Off
            int note = control;
            float freq = 440.0f * pow(2.0f, (note - 69) / 12.0f);

            midiLooper.processMidiMessage(*message);

            state->noteOn = false;

            for (auto& voice : state->voices) {
                if (voice.active && voice.freq == freq) {
                    voice.env.noteOff();
                    voice.filterEnv.noteOff();
                    voice.lfoDepthEnv.noteOff();
                    break;
                }
            }

            std::cout << "Note Off: " << note << std::endl;
        }
    }
}

void setupMidiLooper(SynthState* state) {
    midiLooper.setSendCallback([state](const std::vector<unsigned char>& message) {
        std::cout << "[Looper] Sending MIDI message: ";
        for (unsigned char byte : message) {
            std::cout << (int)byte << " ";
        }
        std::cout << std::endl;

        midiCallback(0.0, const_cast<std::vector<unsigned char>*>(&message), state);
    });

    std::cout << "[Looper] MidiLooper setup complete. Ready to send MIDI messages." << std::endl;
}
