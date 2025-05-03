# Variables
CXX = g++
CXXFLAGS = -std=c++14 -O2
TARGET = synth
SRC = main.cpp ADSR.cpp KeyMapping.cpp KeyboardHandler.cpp AudioCallbackHandler.cpp MidiHandler.cpp Voice.cpp

# RtMidi (dylib)
RTMIDI_DIR = ./rtmidi
RTMIDI_INC = -I$(RTMIDI_DIR)
RTMIDI_LIB = -L$(RTMIDI_DIR)/build -lrtmidi

# PortAudio and SDL2
PA_LIB = -lportaudio
PA_INC = -I/usr/local/include
SDL2_LIB = `sdl2-config --libs` -lSDL2_ttf
SDL2_INC = `sdl2-config --cflags`

# macOS CoreMIDI frameworks
MACOS_FRAMEWORKS = -framework CoreMIDI -framework CoreAudio -framework CoreFoundation

# Règles
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(PA_INC) $(SDL2_INC) $(RTMIDI_INC) -o $(TARGET) $(SRC) $(RTMIDI_LIB) $(PA_LIB) $(SDL2_LIB) $(MACOS_FRAMEWORKS)

run:
	@export DYLD_LIBRARY_PATH=./rtmidi/build:$$DYLD_LIBRARY_PATH && ./$(TARGET)

clean:
	rm -f $(TARGET)
