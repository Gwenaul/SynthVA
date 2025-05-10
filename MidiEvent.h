#ifndef MIDIEVENT_H
#define MIDIEVENT_H

#include <vector>
#include <chrono>

struct MidiEvent {
    std::chrono::milliseconds timestamp;  // Temps de l'événement en ms
    enum Type { NoteOn, NoteOff, ControlChange, PitchBend, ProgramChange, Other } type;  // Types d'événements MIDI
    int note = 0;         // Numéro de la note (pour NoteOn / NoteOff)
    int velocity = 0;     // Vélocité de la note (pour NoteOn)
    int control = 0;      // Contrôle MIDI (pour ControlChange)
    int value = 0;        // Valeur de contrôle (pour ControlChange)
    std::vector<unsigned char> message;  // Message MIDI brut

    MidiEvent() : type(Other) {}
};

#endif // MIDIEVENT_H