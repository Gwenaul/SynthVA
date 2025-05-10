#include <iostream>
#include <vector>
#include <portaudio.h>
#include <SDL2/SDL.h>
#include "SDL2_ttf.h"
#include "Filter.h"
#include "ADSR.h"
#include "PolyBLEP_Saw.h"
#include "PolyBLEP_Square.h"
#include "PolyBLEP_Triangle.h"
#include "NoiseOscillator.h"
#include "MoogFilter.h"
#include "KeyMapping.h"
#include "Waveform.h"
#include "Voice.h"
#include "RtMidi.h"
#include "SynthState.h"
#include "KeyboardHandler.h"
#include "AudioCallbackHandler.h"
#include "MidiHandler.h"
#include <memory>
#include <thread>
#include "MidiLooper.h"

// Définition globale de midiLooper
// C'est la seule définition de cette variable dans tout le programme
MidiLooper midiLooper;

void midiCallback(double deltatime, std::vector<unsigned char> *message, void *userData);
void setupMidiLooper(SynthState* state);

int main() {
    float sampleRate = 44100.0f;
    SynthState state(sampleRate); // Seul constructeur utilisé

    // Objets partagés
    PolyBLEP_Saw osc(sampleRate);
    PolyBLEP_Square oscSquare(sampleRate);
    PolyBLEP_Triangle oscTriangle(sampleRate);
    NoiseOscillator noiseOsc(sampleRate);
    Waveform waveform = Waveform::Saw; // forme d'onde par défaut
    float volume = 0.2f;
    int currentOctave = 4;
    bool noteOn = false;
    ADSR env(sampleRate);
    MoogFilter moogFilter(sampleRate);
    moogFilter.setCutoff(1000.0f);
    moogFilter.setResonance(0.7f);
    
    // Créer un objet global ou local selon l'architecture
    RtMidiIn* midiin = new RtMidiIn();
    
    try {
        midiin->openPort(1);  // 0 = premier périphérique MIDI détecté
        midiin->setCallback(&midiCallback, &state); // `state` est un pointeur vers le SynthState
        midiin->ignoreTypes(false, false, false);   // N'ignore aucun message
    } catch (RtMidiError& error) {
        error.printMessage();
        exit(EXIT_FAILURE);
    }

    setupMidiLooper(&state);

    // Affectation dans le state
    state.osc = &osc;
    state.oscSquare = &oscSquare;
    state.oscTriangle = &oscTriangle;
    state.noiseOsc = &noiseOsc;
    state.waveform = &waveform;
    state.volume = volume;
    state.octave = currentOctave;
    state.noteOn = noteOn;
    state.env = env;
    state.moogFilter = moogFilter;

    // Initialisation des voix
    state.voices.reserve(state.maxVoices);
    for (int i = 0; i < state.maxVoices; ++i) {
        state.voices.emplace_back(sampleRate);
    }

    // PortAudio
    PaError err;
    PaStream* stream;

    err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        return 1;
    }

    err = Pa_OpenDefaultStream(&stream, 0, 2, paFloat32, sampleRate, 256, AudioCallbackHandler::audioCallback, &state);
    if (err != paNoError) {
        std::cerr << "PortAudio open stream error: " << Pa_GetErrorText(err) << std::endl;
        return 1;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        std::cerr << "PortAudio start stream error: " << Pa_GetErrorText(err) << std::endl;
        return 1;
    }

    // SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (TTF_Init() == -1) {
        std::cerr << "TTF_Init Error: " << TTF_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Synth Visual", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 920, 480, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    TTF_Font* font = TTF_OpenFont("/Library/Fonts/FiraCode-Regular.ttf", 16);

    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return 1;
    }

    std::cout << "Press keys (a, s, d, etc.) to play notes. Press ESC to quit." << std::endl;
    bool quit = false;
    SDL_Event event;

    // Main loop
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            handleKeyboard(state, event, quit);
        }
        // Mettre à jour le looper - c'est la seule instance de MidiLooper qui existe
        midiLooper.update();
        renderUI(renderer, font, state);
        SDL_Delay(16); // ~60 FPS
    }

    // Cleanup
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    SDL_Quit();

    return 0;
}