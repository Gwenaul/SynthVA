#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <sstream>
#include "SynthState.h"

void renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y) {
    SDL_Color color = {255, 255, 255};  // White
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dest = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void renderADSR(SDL_Renderer* renderer, SynthState& state, int x, int y, int width, int height) {
    float a = state.env.getAttack();
    float d = state.env.getDecay();
    float s = state.env.getSustain(); // niveau
    float r = state.env.getRelease();

    // Total duration for scaling x (arbitrary sustain duration)
    float sustainDuration = 0.2f; // fixed value just for drawing
    float totalTime = a + d + sustainDuration + r;

    // Time points
    float t0 = 0;
    float t1 = t0 + a;
    float t2 = t1 + d;
    float t3 = t2 + sustainDuration;
    float t4 = t3 + r;

    // Normalize time to fit width
    auto timeToX = [&](float t) -> int {
        return x + static_cast<int>((t / totalTime) * width);
    };

    auto levelToY = [&](float level) -> int {
        return y + static_cast<int>((1.0f - level) * height);
    };

    // Points
    SDL_Point points[5];
    points[0] = {timeToX(t0), levelToY(0.0f)};
    points[1] = {timeToX(t1), levelToY(1.0f)};
    points[2] = {timeToX(t2), levelToY(s)};
    points[3] = {timeToX(t3), levelToY(s)};
    points[4] = {timeToX(t4), levelToY(0.0f)};

    // Set color and draw the ADSR line
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Green
    SDL_RenderDrawLines(renderer, points, 5);
}


void renderUI(SDL_Renderer* renderer, TTF_Font* font, SynthState& state) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(renderer);

    std::string waveformStr;
    switch (*state.waveform) {
        case Waveform::Saw: waveformStr = "Saw"; break;
        case Waveform::Square: waveformStr = "Square"; break;
        case Waveform::Triangle: waveformStr = "Triangle"; break;
        case Waveform::Noise: waveformStr = "Noise"; break;
    }

    std::ostringstream oss;
    oss << "Waveform: " << waveformStr;
    renderText(renderer, font, oss.str(), 10, 10);

    // Show active note (optional, just shows first active one)
    for (auto& voice : state.voices) {
        if (voice.active) {
            std::ostringstream noteStr;
            noteStr << "Freq: " << voice.freq << " Hz";
            renderText(renderer, font, noteStr.str(), 10, 40);
            std::ostringstream adsrStr;
            adsrStr << "A:" << state.env.getAttack()
                    << " D:" << state.env.getDecay()
                    << " S:" << state.env.getSustain()
                    << " R:" << state.env.getRelease();
            renderText(renderer, font, adsrStr.str(), 10, 140); // Affiche ligne suivante

            break;
        }
    }

    // Volume
    std::ostringstream volStr;
    volStr << "Volume: " << static_cast<int>(state.volume * 100) << "%";
    renderText(renderer, font, volStr.str(), 10, 70);

    renderText(renderer, font, "Press ESC to quit", 10, 110);
    
    renderADSR(renderer, state, 300, 20, 200, 100);  // x, y, width, height

    SDL_RenderPresent(renderer);
}
