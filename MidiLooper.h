#ifndef MIDILOOPER_H
#define MIDILOOPER_H

#include <set>
#include <vector>
#include <cstdint>
#include <chrono>
#include <functional>
#include "MidiEvent.h"

class MidiLooper {

public:
    MidiLooper();
    void processMidiMessage(const std::vector<unsigned char>& message);
    void update(); // à appeler régulièrement dans la boucle audio/MIDI
    void resetLoop();
    void setSendCallback(std::function<void(const std::vector<unsigned char>&)> cb);
    void send(const std::vector<unsigned char>& message);

private:
    enum State {
        Idle,
        Recording,
        Playing
    };

    State state;
    std::chrono::steady_clock::time_point loopStartTime;
    std::chrono::milliseconds loopDuration;
    std::vector<MidiEvent> recordedEvents;
    std::chrono::steady_clock::time_point playbackStartTime;
    std::function<void(const std::vector<unsigned char>&)> sendCallback;
    std::set<unsigned char> activeNotes;
    std::chrono::milliseconds previousLoopTime{0};
    std::chrono::milliseconds lastProcessedTime{0};
    std::chrono::steady_clock::time_point lastLoopCheckTime;

    void startRecording();
    void stopRecording();
    void startPlayback();
    void stopPlayback();

    void handleControlChange(unsigned char controller, unsigned char value);
    bool isControlChange(const std::vector<unsigned char>& msg, unsigned char ctrlNum);
};

#endif // MIDILOOPER_H


