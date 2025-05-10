#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <sstream>
#include "SynthState.h"

// Valeurs par défaut pour A, D, S, R
float a = 0.01f, d = 0.1f, s = 0.1f, r = 0.2f;
bool foundVoice = false;
float verticalScale = 8.0f;

void renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y) {
    SDL_Color color = {255, 255, 255};  // White
    // SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dest = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void renderWaveform(SDL_Renderer* renderer, const float* buffer, int size, int index, int x, int y, int width, int height, float verticalScale) {
    SDL_SetRenderDrawColor(renderer, 0, 128, 255, 255);  // Bleu clair

    SDL_Point points[size];
    for (int i = 0; i < size; ++i) {
        int bufferIndex = (index + i) % size;  // Accès circulaire
        float sample = buffer[bufferIndex];

        int px = x + (i * width) / size;
        int py = y + height / 2 - static_cast<int>(sample * (height / 2) * verticalScale);  // Échelle verticale

        points[i] = { px, py };
    }

    SDL_RenderDrawLines(renderer, points, size);
}

void renderWaveformPeriod(SDL_Renderer* renderer, const float* buffer, int size, int index, int x, int y, int width, int height, float verticalScale, int voiceIndex) {
    // Reconstruire le buffer circulaire en linéaire
    std::vector<float> samples(size);
    for (int i = 0; i < size; ++i) {
        samples[i] = buffer[(index + i) % size];
    }

    // Chercher deux fronts montants (zéro-crossings)
    int start = -1, end = -1;
    for (int i = 1; i < size; ++i) {
        if (samples[i - 1] < 0.0f && samples[i] >= 0.0f) {
            if (start == -1)
                start = i;
            else {
                end = i;
                break;
            }
        }
    }

    // Si on n’a pas trouvé deux points : abandonner
    if (start == -1 || end == -1 || end - start < 2)
        return;

    int periodLength = end - start;
    std::vector<SDL_Point> points(periodLength);
    for (int i = 0; i < periodLength; ++i) {
        float value = samples[start + i];
        int px = x + (i * width) / periodLength;
        int reducedHeight = height / 32;
        int py = y + static_cast<int>((1.0f - value * verticalScale) * (reducedHeight / 2)) - reducedHeight / 4; // centré verticalement
        points[i] = {px, py};
    }

    // Dessiner chaque voix avec une couleur et un décalage
    // Par exemple, ajouter l'index de la voix pour changer la couleur et appliquer un décalage
    
    // Couleurs : Jaune pâle pour la première, rouge pâle pour la seconde, etc.
    Uint8 red = 255, green = 255, blue = 0;  // Couleur jaune pâle initiale

    // Calculer la couleur en fonction de l'indice de la voix
    switch (voiceIndex) {
        case 0: red = 255; green = 255; blue = 0; break;     // Jaune
        case 1: red = 255; green = 100; blue = 100; break;   // Rouge pâle
        case 2: red = 255; green = 165; blue = 0; break;     // Orange
        case 3: red = 100; green = 255; blue = 100; break;   // Vert clair
        case 4: red = 100; green = 100; blue = 255; break;   // Bleu clair
        case 5: red = 200; green = 100; blue = 200; break;   // Violet
        default: red = 200; green = 200; blue = 200; break;  // Gris par défaut
    }

    SDL_SetRenderDrawColor(renderer, red, green, blue, 255);  // Définir la couleur en fonction de l'indice de la voix
    SDL_RenderDrawLines(renderer, points.data(), points.size());
}

std::string createGauge(float value, int width = 10) {
    int filled = static_cast<int>(value * width);
    std::string gauge;

    // Utiliser des chaînes UTF-8 pour ▓ et ░
    std::string filledChar = u8"▓";
    std::string emptyChar  = u8"░";

    for (int i = 0; i < filled; ++i) gauge += filledChar;
    for (int i = filled; i < width; ++i) gauge += emptyChar;

    int percent = static_cast<int>(value * 100);
    std::ostringstream oss;
    oss << gauge << " (" << percent << "%)";
    return oss.str();
}

std::string createVerticalBlockGauge(float value, int height = 8) {
    const std::string levels[8] = {
        u8"▁", u8"▂", u8"▃", u8"▄", u8"▅", u8"▆", u8"▇", u8"█"
    };

    int filled = static_cast<int>(value * height);
    std::ostringstream oss;

    for (int i = 0; i < height; ++i) {
        oss << (i < filled ? levels[i] : " ");
    }

    return oss.str();
}

std::string createSingleBlockGauge(float value) {
    const std::string levels[8] = {
        u8"▁", u8"▂", u8"▃", u8"▄", u8"▅", u8"▆", u8"▇", u8"█"
    };

    int index = static_cast<int>(value * 7.99f); // Multiplie par 7 pour avoir un index 0–7
    return levels[index];
}

std::string utf8_from_utf32(char32_t ch) {
    std::string result;
    if (ch <= 0x7F) {
        result += static_cast<char>(ch);
    } else if (ch <= 0x7FF) {
        result += static_cast<char>(0xC0 | ((ch >> 6) & 0x1F));
        result += static_cast<char>(0x80 | (ch & 0x3F));
    } else if (ch <= 0xFFFF) {
        result += static_cast<char>(0xE0 | ((ch >> 12) & 0x0F));
        result += static_cast<char>(0x80 | ((ch >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (ch & 0x3F));
    } else {
        result += static_cast<char>(0xF0 | ((ch >> 18) & 0x07));
        result += static_cast<char>(0x80 | ((ch >> 12) & 0x3F));
        result += static_cast<char>(0x80 | ((ch >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (ch & 0x3F));
    }
    return result;
}

std::string createFramedGauge(float value) {
    const int totalBlocks = 6;

    // Clamp la valeur
    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;

    // Combien de blocs doivent être "pleins" ?
    int filledBlocks = static_cast<int>(value * totalBlocks);

    std::string gauge;

    for (int i = 0; i < totalBlocks; ++i) {
        bool isFilled = (i < filledBlocks);
        char32_t codepoint;

        if (i == 0) {
            codepoint = isFilled ? 0xEE03 : 0xEE00;  // début
        } else if (i == totalBlocks - 1) {
            codepoint = isFilled ? 0xEE05 : 0xEE02;  // fin
        } else {
            codepoint = isFilled ? 0xEE04 : 0xEE01;  // milieu
        }

        gauge += utf8_from_utf32(codepoint);
    }

    return gauge;
}

std::string createFramedGaugeWithLevel(float value) {
    const int totalBlocks = 6;

    // Clamp la valeur
    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;

    int filledBlocks = static_cast<int>(value * totalBlocks);
    std::string gauge;

    for (int i = 0; i < totalBlocks; ++i) {
        char32_t codepoint;
        bool isFilled = (i < filledBlocks);

        if (i == 0) {
            codepoint = isFilled ? 0xEE03 : 0xEE00;
        } else if (i == totalBlocks - 1) {
            codepoint = isFilled ? 0xEE05 : 0xEE02;
        } else {
            codepoint = isFilled ? 0xEE04 : 0xEE01;
        }

        gauge += utf8_from_utf32(codepoint);
    }

    // Ajouter le bloc indicateur dynamique (6 niveaux : EE06 à EE0B)
    const int dynamicIndex = static_cast<int>(value * 5.99f);  // Index de 0 à 5
    char32_t dynamicBlock = 0xEE06 + dynamicIndex;
    gauge += utf8_from_utf32(dynamicBlock);

    return gauge;
}

void renderADSR(SDL_Renderer* renderer, TTF_Font* font, SynthState& state, int x, int y, int width, int height) {
    bool foundVoice = false;
    
    for (auto& voice : state.voices) {
        if (voice.active) {
            a = voice.env.getAttack();
            d = voice.env.getDecay();
            s = voice.env.getSustain();
            r = voice.env.getRelease();
            foundVoice = true;
            break;
        }
    }

    if (!foundVoice) {
        return;
    }

    // Total duration for scaling x (arbitrary sustain duration)
    float sustainDuration = 0.2f; // Fixed value for drawing
    float totalTime = a + d + sustainDuration + r;

    // Points de temps
    float t0 = 0;
    float t1 = t0 + a;
    float t2 = t1 + d;
    float t3 = t2 + sustainDuration;
    float t4 = t3 + r;

    // Normaliser le temps pour l'adapter à la largeur de l'écran
    auto timeToX = [&](float t) -> int {
        return x + static_cast<int>((t / totalTime) * width);
    };

    // Convertir les niveaux en Y pour l'affichage
    auto levelToY = [&](float level) -> int {
        return y + static_cast<int>((1.0f - level) * height);
    };

    // Points pour dessiner la courbe ADSR
    SDL_Point points[5];
    points[0] = {timeToX(t0), levelToY(0.0f)};
    points[1] = {timeToX(t1), levelToY(1.0f)};
    points[2] = {timeToX(t2), levelToY(s)};
    points[3] = {timeToX(t3), levelToY(s)};
    points[4] = {timeToX(t4), levelToY(0.0f)};

    // Dessiner la courbe ADSR en vert
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Vert
    SDL_RenderDrawLines(renderer, points, 5);
    
    // // Affichage des jauges ADSR
    renderText(renderer, font, "A", 790, 130);
    renderText(renderer, font, createSingleBlockGauge(a / 2.0f), 790, 110);// x, y

    renderText(renderer, font, "D", 810, 130);
    renderText(renderer, font, createSingleBlockGauge(d / 3.0f), 810, 110);// x, y

    renderText(renderer, font, "S", 830, 130);
    renderText(renderer, font, createSingleBlockGauge(s / 1.0f), 830, 110);// x, y

    renderText(renderer, font, "R", 850, 130);
    renderText(renderer, font, createSingleBlockGauge(r / 5.0f), 850, 110);// x, y
}

void renderUI(SDL_Renderer* renderer, TTF_Font* font, SynthState& state) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(renderer);

    // Afficher le nom de la forme d'onde
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
    int offsetX = 0;
    int offsetY = 0;

    // Afficher la fréquence active
    for (auto& voice : state.voices) {
        if (voice.active) {
            std::ostringstream noteStr;
            noteStr << "Freq: " << voice.freq << " Hz";
            renderText(renderer, font, noteStr.str(), 10, 40);

            // Affichage de la jauge de filtre
            float normCutoff = voice.moogFilter.getCutoff() / 20000.0f;
            std::ostringstream cutoffStr;
            cutoffStr << "Cutoff:       " << createGauge(normCutoff, 20)
                    // << " " << static_cast<int>(normCutoff) << " Hz"
                    ;
            renderText(renderer, font, cutoffStr.str(), 10, 320);

            // Jauge pour résonance
            float normResonance = voice.moogFilter.getResonance() / 4.0f;
            std::ostringstream resonanceStr;
            resonanceStr << "Resonance:    " << createGauge(normResonance, 20)
                        // << " " << static_cast<int>(normResonance * 100) << "%"
                        ;
            renderText(renderer, font, resonanceStr.str(), 10, 350);

            // Jauge pour lfoDepth
            float normlfoDepth = state.lfoDepth / 100.0f;
            std::ostringstream lfoDepthStr;
            lfoDepthStr << "lfoDepth:     " << createGauge(normlfoDepth, 20)
                        // << " " << static_cast<int>(state.lfoDepth) << " %"
                        ;
            renderText(renderer, font, lfoDepthStr.str(), 10, 380);// x, y

            // Jauge pour lfoFrequency
            float normlfoFrequency = state.lfoFrequency / 20.0f;
            std::ostringstream lfoFrequencyStr;
            lfoFrequencyStr << "lfoFrequency: " << createGauge(normlfoFrequency, 20)
                        // << " " << static_cast<int>(normlfoFrequency) << "Hz"
                        ;
            renderText(renderer, font, lfoFrequencyStr.str(), 10, 410);// x, y

            renderWaveform(renderer, state.renderBuffer, SynthState::WaveformBufferSize, state.renderBufferIndex, 10, 130, 760, 200, verticalScale);// x, y, width, height        
            for (int i = 0; i < 6; ++i) {
                renderWaveformPeriod(renderer, state.renderBuffers[i], SynthState::WaveformBufferSize,
                                    state.renderBufferIndices[i], 450 + offsetX, 400 + offsetY, 400, 300, verticalScale, i);
                offsetX += 10;  // Décale chaque voix en X
                offsetY -= 10;  // Décale chaque voix en Y
            }

            int x = 790;  // Position X de départ (ajuster selon la largeur de fenêtre)
            int baseY = 200;  // Position Y de départ (bas de la fenêtre)
            // SAW
            renderText(renderer, font, "Saw", x, baseY);// x, y
            renderText(renderer, font, createFramedGauge(state.mixSaw), x + 40, baseY);// x, y
            // SQUARE
            renderText(renderer, font, "Sq", x, baseY + 20);// x, y
            renderText(renderer, font, createFramedGauge(state.mixSquare), x + 40, baseY + 20);// x, y
            // TRIANGLE
            renderText(renderer, font, "Tri", x, baseY + 40);// x, y
            renderText(renderer, font, createFramedGauge(state.mixTriangle), x + 40, baseY + 40);// x, y
            // NOISE
            renderText(renderer, font, "N", x, baseY + 60);// x, y
            renderText(renderer, font, createFramedGauge(state.mixNoise), x + 40, baseY + 60);// x, y
            renderText(renderer, font, createFramedGaugeWithLevel(state.mixSaw), x + 40, baseY);// x, y
            renderText(renderer, font, createFramedGaugeWithLevel(state.mixSquare), x + 40, baseY + 20);// x, y
            renderText(renderer, font, createFramedGaugeWithLevel(state.mixTriangle), x + 40, baseY + 40);// x, y
            renderText(renderer, font, createFramedGaugeWithLevel(state.mixNoise), x + 40, baseY + 60);// x, y    
            break;
        }
    }

    // Volume
    std::ostringstream volStr;
    volStr << "Volume: " << static_cast<int>(state.volume * 100) << "%";
    renderText(renderer, font, volStr.str(), 790, 70);// x, y
    renderText(renderer, font, createVerticalBlockGauge(state.volume), 790, 40); // x, y

    renderText(renderer, font, "Press ESC to quit", 10, 110);// x, y
    
    renderADSR(renderer, font, state, 300, 20, 400, 100);  // x, y, width, height

    SDL_RenderPresent(renderer);
}

