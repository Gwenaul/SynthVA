#include "MidiLooper.h"
#include <iostream>

MidiLooper::MidiLooper()
    : state(Idle), loopDuration(0)
{}

void MidiLooper::processMidiMessage(const std::vector<unsigned char>& message) {
    if (message.size() < 3) return;

    unsigned char status = message[0];
    unsigned char type = status & 0xF0;

    // Gérer les contrôleurs immédiatement
    if (type == 0xB0) {
        handleControlChange(message[1], message[2]);
    }

    if (state == Recording) {
        if (type == 0xB0 && (message[1] == 119 || message[1] == 118 || message[1] == 117)) {
            return; // Ne pas enregistrer les controlChange Transport message
        }
        auto now = std::chrono::steady_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now - loopStartTime);

        MidiEvent event;
        event.timestamp = timestamp;
        event.message = message;

        switch (type) {
            case 0x90: // Note On
                if (message[2] == 0) {
                    event.type = MidiEvent::NoteOff;
                    event.note = message[1];
                    activeNotes.erase(message[1]);
                } else {
                    event.type = MidiEvent::NoteOn;
                    event.note = message[1];
                    event.velocity = message[2];
                    activeNotes.insert(message[1]);
                }
                break;
            case 0x80: // Note Off
                event.type = MidiEvent::NoteOff;
                event.note = message[1];
                activeNotes.erase(message[1]);
                break;
            case 0xB0: // Control Change
                event.type = MidiEvent::ControlChange;
                event.control = message[1];
                event.value = message[2];
                break;
            default:
                event.type = MidiEvent::Other;
                break;
        }

        recordedEvents.push_back(event);
    }
}

void MidiLooper::handleControlChange(unsigned char controller, unsigned char value) {
    if (value == 0) return; // on ne traite que les appuis

    switch (controller) {
        case 119: // Record
            if (state != Recording) startRecording();
            else stopRecording();
            break;
        case 118: // Play
            if (state == Idle && !recordedEvents.empty()) startPlayback();
            break;
        case 117: // Stop
            stopPlayback();
            break;
        case 115: // Left
            std::cout << "[Looper] << Left" << std::endl;
            break;
        case 116: // Right
            std::cout << "[Looper] >> Right" << std::endl;
            break;
    }
}

void MidiLooper::startRecording() {
    std::cout << "[Looper] Recording started" << std::endl;
    recordedEvents.clear();
    loopStartTime = std::chrono::steady_clock::now();
    state = Recording;
    std::cout << "[Looper] State changed to: " << state << std::endl; // Vérifier le changement d'état
}

void MidiLooper::stopRecording() {
    auto now = std::chrono::steady_clock::now();
    loopDuration = std::chrono::duration_cast<std::chrono::milliseconds>(now - loopStartTime);
    std::cout << "[Looper] Recording stopped, duration: " << loopDuration.count() << "ms" << std::endl;
    std::cout << "[Looper] State before starting playback: " << state << std::endl;

    // Forcer les notes actives à s'arrêter
    for (auto note : activeNotes) {
        MidiEvent offEvent;
        offEvent.timestamp = loopDuration;
        offEvent.type = MidiEvent::NoteOff;
        offEvent.note = note;
        offEvent.message = {0x80, note, 0}; // Note Off, canal 0
        recordedEvents.push_back(offEvent);
    }
    activeNotes.clear(); // Réinitialisation pour la prochaine boucle

    std::sort(recordedEvents.begin(), recordedEvents.end(),
    [](const MidiEvent& a, const MidiEvent& b) {
        return a.timestamp < b.timestamp;
    });

    startPlayback();
}


void MidiLooper::startPlayback() {
    std::sort(recordedEvents.begin(), recordedEvents.end(),
        [](const MidiEvent& a, const MidiEvent& b) {
            return a.timestamp < b.timestamp;
        });
    std::cout << "[Looper] Playback started" << std::endl;
    playbackStartTime = std::chrono::steady_clock::now();
    lastLoopCheckTime = playbackStartTime;
    state = Playing;
    std::cout << "[Looper] State changed to: " << state << std::endl; // Vérifier le changement d'état
}

void MidiLooper::stopPlayback() {
    std::cout << "[Looper] Playback stopped" << std::endl;
    state = Idle;
    std::cout << "[Looper] State changed to: " << state << std::endl; // Vérifier le changement d'état
}

void MidiLooper::setSendCallback(std::function<void(const std::vector<unsigned char>&)> cb) {
    sendCallback = cb;
}

void MidiLooper::send(const std::vector<unsigned char>& message) {
    std::cout << "[Looper] Sending MIDI message from looper..." << std::endl;
    if (sendCallback) {
        sendCallback(message);
    } else {
        std::cerr << "[Looper] Warning: No send callback set!" << std::endl;
    }
}

void MidiLooper::update() {
    if (state != Playing) return;

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - playbackStartTime);
    auto loopTime = elapsed % loopDuration;

    // Détection de redémarrage de boucle (tour suivant)
    auto loopCyclesNow = elapsed / loopDuration;
    auto lastElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(lastLoopCheckTime - playbackStartTime);
    auto lastLoopCycles = lastElapsed / loopDuration;

    if (loopCyclesNow > lastLoopCycles) {
        std::cout << "[Looper] Loop restarted!" << std::endl;
    }

    // Envoi des événements MIDI dans la fenêtre [lastProcessedTime, loopTime]
    for (const auto& event : recordedEvents) {
        if (event.timestamp > lastProcessedTime && event.timestamp <= loopTime) {
            if (event.type == MidiEvent::ControlChange &&
                (event.control == 119 || event.control == 118 || event.control == 117)) {
                continue; // Ne pas rejouer les contrôles de transport
            }
            std::cout << "[Looper] Sending event message for note: " << (int)event.note << std::endl;
            send(event.message);
        }
    }

    // Mise à jour de la position de la lecture
    lastProcessedTime = loopTime;
    lastLoopCheckTime = now;
}

void MidiLooper::resetLoop() {
    recordedEvents.clear();
    loopDuration = std::chrono::milliseconds(0);
    state = Idle;
}