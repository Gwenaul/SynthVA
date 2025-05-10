#include "MidiHandler.h"
#include <iostream>
#include <vector>
#include <cmath>

// Clamp en C++14
template<typename T>
T clamp(T value, T low, T high) {
    if (value < low) return low;
    if (value > high) return high;
    return value;
}

// Déclaration externe au lieu de définition
// Cette ligne indique que midiLooper existe ailleurs (dans le main.cpp)
extern MidiLooper midiLooper;

float computeEffectiveResonance(float userRes, float cutoff) {
    float normCutoff = clamp((cutoff - 100.0f) / (20000.0f - 100.0f), 0.0f, 1.0f);
    float adjustedRes = userRes * (1.2f - normCutoff);
    return clamp(adjustedRes, 0.0f, 4.0f);
}

void midiCallback(double deltatime, std::vector<unsigned char> *message, void *userData) {
    SynthState* state = static_cast<SynthState*>(userData);

    if (message->size() >= 3) {
        unsigned char status = message->at(0);
        unsigned char control = message->at(1);
        unsigned char value = message->at(2);

        // Vérifier que c'est un Control Change (status 0xB0 à 0xBF)
        if ((status & 0xF0) == 0xB0) {

            //pour capter les commandes transport
            midiLooper.processMidiMessage(*message);

            // Mapping :
            if (control == 1) {
                float cutoff = 100.0f * std::pow(200.0f, value / 127.0f);
                state->currentCutoff = cutoff;

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
            } else if (control == 3) { // LFO depth
                state->lfoEnabled = (value >= 4);
                // std::cout << "[LFO] " << (state->lfoEnabled ? "Enabled" : "Disabled") << std::endl;
                state->lfoDepth = clamp(value / 127.0f, 0.0f, 1.0f) * 50.0f;
                // std::cout << "[LFO Depth] " << state->lfoDepth << "%" << std::endl;
            } else if (control == 4) { // LFO Frequency
                float freq = 0.1f + (value / 127.0f) * 20.0f;
                state->lfoFrequency = freq;
                state->lfo.setFrequency(freq);
                // std::cout << "[LFO Frequency] " << freq << " Hz" << std::endl;
            } else if (control == 6) { // Volume
                state->volume = clamp(value / 127.0f, 0.0f, 1.0f);
                // std::cout << "[Volume] " << (state->volume * 100.0f) << "%" << std::endl;
            } else if (control == 7) { // Saw mix
                state->mixSaw = clamp(value / 127.0f, 0.0f, 1.0f);
                // std::cout << "[Osc Mix] Saw: " << state->mixSaw << std::endl;
            } else if (control == 8) { // Square mix
                state->mixSquare = clamp(value / 127.0f, 0.0f, 1.0f);
                // std::cout << "[Osc Mix] Square: " << state->mixSquare << std::endl;
            } else if (control == 9) { // Triangle mix
                state->mixTriangle = clamp(value / 127.0f, 0.0f, 1.0f);
                // std::cout << "[Osc Mix] Triangle: " << state->mixTriangle << std::endl;
            } else if (control == 10) { // Noise mix
                state->mixNoise = clamp(value / 127.0f, 0.0f, 1.0f);
                // std::cout << "[Osc Mix] Noise: " << state->mixNoise << std::endl;
            } else if (control == 11) { // Attack
                // Convertir la valeur MIDI en secondes (0-127 -> 0.001-2.0 secondes)
                float attackTime = 0.001f + (value / 127.0f) * 1.999f;
                
                // Mettre à jour l'ADSR pour toutes les voix actuelles ET futures
                state->attackTime = attackTime;
                
                // Mise à jour pour toutes les voix existantes
                for (auto& voice : state->voices) {
                    voice.env.setAttack(attackTime);
                }
                // std::cout << "[Envelope] Attack: " << attackTime << " sec" << std::endl;
            } else if (control == 12) { // Decay
                // Convertir la valeur MIDI en secondes (0-127 -> 0.001-3.0 secondes)
                float decayTime = 0.001f + (value / 127.0f) * 2.999f;
                
                // Mettre à jour l'ADSR pour toutes les voix actuelles ET futures
                state->decayTime = decayTime;
                
                // Mise à jour pour toutes les voix existantes
                for (auto& voice : state->voices) {
                    voice.env.setDecay(decayTime);
                }
                // std::cout << "[Envelope] Decay: " << decayTime << " sec" << std::endl;
            } else if (control == 13) { // Sustain
                // Convertir la valeur MIDI en niveau (0-127 -> 0.0-1.0)
                float sustainLevel = value / 127.0f;
                
                // Mettre à jour l'ADSR pour toutes les voix actuelles ET futures
                state->sustainLevel = sustainLevel;
                
                // Mise à jour pour toutes les voix existantes
                for (auto& voice : state->voices) {
                    voice.env.setSustain(sustainLevel);
                }
                // std::cout << "[Envelope] Sustain: " << sustainLevel << std::endl;
            } else if (control == 14) { // Release
                // Convertir la valeur MIDI en secondes (0-127 -> 0.001-5.0 secondes)
                float releaseTime = 0.001f + (value / 127.0f) * 4.999f;
                
                // Mettre à jour l'ADSR pour toutes les voix actuelles ET futures
                state->releaseTime = releaseTime;
                
                // Mise à jour pour toutes les voix existantes
                for (auto& voice : state->voices) {
                    voice.env.setRelease(releaseTime);
                }
                // std::cout << "[Envelope] Release: " << releaseTime << " sec" << std::endl;
            } else if (control == 5) { // LFO Enable/Disable
                for (auto& voice : state->voices) {
                    if (value < 33) {
                        voice.setNoiseType(NoiseOscillator::NoiseType::PINK);
                    } else if (value >= 33 && value < 66) {
                        voice.setNoiseType(NoiseOscillator::NoiseType::WHITE);
                    } else if (value >= 66) {
                        voice.setNoiseType(NoiseOscillator::NoiseType::BROWN);
                    }
                }
            }            
        }

        if ((status & 0xF0) == 0x90 && value > 0) {  // Note On
            int note = control;
            float freq = 440.0f * pow(2.0f, (note - 69) / 12.0f); // MIDI note to frequency

            midiLooper.processMidiMessage(*message);

            // Vérifier si une voix joue déjà cette note
            bool found = false;
            for (auto& voice : state->voices) {
                if (voice.active && voice.freq == freq) {
                    voice.env.noteOn(); // retrigger env

                    found = true;
                    // std::cout << "Retriggering existing voice for Note: " << note << std::endl;
                    break;
                }
            }

            // Si aucune voix existante ne joue cette note, en créer une nouvelle
            if (!found) {
                // Vérifier si on a atteint le nombre max de voix
                if (state->voices.size() >= state->maxVoices) {
                    // Trouver la voix la plus ancienne en mode RELEASE ou IDLE
                    int oldestVoiceIndex = -1;
                    for (size_t i = 0; i < state->voices.size(); i++) {
                        if (!state->voices[i].isActive()) {
                            oldestVoiceIndex = i;
                            break;
                        }
                    }

                    // Si toutes les voix sont actives, on remplace simplement la première
                    if (oldestVoiceIndex == -1) oldestVoiceIndex = 0;

                    // Réutiliser cette voix
                    auto& reusedVoice = state->voices[oldestVoiceIndex];
                    reusedVoice.freq = freq;
                    reusedVoice.active = true;

                    // Appliquer les bons paramètres ADSR
                    reusedVoice.env.setAttack(state->attackTime);
                    reusedVoice.env.setDecay(state->decayTime);
                    reusedVoice.env.setSustain(state->sustainLevel);
                    reusedVoice.env.setRelease(state->releaseTime);

                    reusedVoice.env.noteOn();
                } else {
                    // Créer une nouvelle voix
                    Voice voice(state->sampleRate);

                    // Appliquer les paramètres ADSR actuels
                    voice.env.setAttack(state->attackTime);
                    voice.env.setDecay(state->decayTime);
                    voice.env.setSustain(state->sustainLevel);
                    voice.env.setRelease(state->releaseTime);

                    voice.freq = freq;
                    voice.active = true;
                    voice.env.noteOn();
                    state->voices.push_back(voice);
                }
            }
            std::cout << "Note On: " << note << " -> " << freq << " Hz" << std::endl;
        }

        else if ((status & 0xF0) == 0x80 || ((status & 0xF0) == 0x90 && value == 0)) {  // Note Off
            int note = control;
            float freq = 440.0f * pow(2.0f, (note - 69) / 12.0f);

            midiLooper.processMidiMessage(*message);

            state->noteOn = false;
            
            // Trouver et relâcher la voix correspondante
            for (auto& voice : state->voices) {
                if (voice.active && voice.freq == freq) {

                    voice.env.noteOff();
                    break;
                }
            }

            std::cout << "Note Off: " << note << std::endl;
        }
    }
}

void setupMidiLooper(SynthState* state) {
    midiLooper.setSendCallback([state](const std::vector<unsigned char>& message) {
        // Affiche un message lorsque un message MIDI est envoyé
        std::cout << "[Looper] Sending MIDI message: ";
        for (unsigned char byte : message) {
            std::cout << (int)byte << " ";
        }
        std::cout << std::endl;

        // Réinjecte dans le synthé, comme si on recevait le message en temps réel
        midiCallback(0.0, const_cast<std::vector<unsigned char>*>(&message), state);
    });

    std::cout << "[Looper] MidiLooper setup complete. Ready to send MIDI messages." << std::endl;
}