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

    float a = 0.0f, d = 0.0f, s = 0.0f, r = 0.0f;

    for (auto& voice : state.voices) {
        if (voice.active) {
            switch (state.oscControlGroup) {
                case 2: // Filter envelope
                    a = voice.filterEnv.getAttack();
                    d = voice.filterEnv.getDecay();
                    s = voice.filterEnv.getSustain();
                    r = voice.filterEnv.getRelease();
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Rouge
                    break;
                case 3: // Pitch envelope
                    a = voice.lfoDepthEnv.getAttack();
                    d = voice.lfoDepthEnv.getDecay();
                    s = voice.lfoDepthEnv.getSustain();
                    r = voice.lfoDepthEnv.getRelease();
                    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Bleu
                    break;
                default: // Volume envelope (cas 0 ou 1)
                    a = voice.env.getAttack();
                    d = voice.env.getDecay();
                    s = voice.env.getSustain();
                    r = voice.env.getRelease();
                    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Vert
                    break;
            }
            foundVoice = true;
            break;
        }
    }

    if (!foundVoice) return;

    float sustainDuration = 0.2f;
    float totalTime = a + d + sustainDuration + r;

    float t0 = 0;
    float t1 = t0 + a;
    float t2 = t1 + d;
    float t3 = t2 + sustainDuration;
    float t4 = t3 + r;

    auto timeToX = [&](float t) -> int {
        return x + static_cast<int>((t / totalTime) * width);
    };

    auto levelToY = [&](float level) -> int {
        return y + static_cast<int>((1.0f - level) * height);
    };

    SDL_Point points[5];
    points[0] = {timeToX(t0), levelToY(0.0f)};
    points[1] = {timeToX(t1), levelToY(1.0f)};
    points[2] = {timeToX(t2), levelToY(s)};
    points[3] = {timeToX(t3), levelToY(s)};
    points[4] = {timeToX(t4), levelToY(0.0f)};

    SDL_RenderDrawLines(renderer, points, 5);

    // === Affichage des jauges ADSR ===
    renderText(renderer, font, "A", 790, 130);
    renderText(renderer, font, createSingleBlockGauge(a / 2.0f), 790, 110);

    renderText(renderer, font, "D", 810, 130);
    renderText(renderer, font, createSingleBlockGauge(d / 3.0f), 810, 110);

    renderText(renderer, font, "S", 830, 130);
    renderText(renderer, font, createSingleBlockGauge(s / 1.0f), 830, 110);

    renderText(renderer, font, "R", 850, 130);
    renderText(renderer, font, createSingleBlockGauge(r / 5.0f), 850, 110);
}

void renderUI(SDL_Renderer* renderer, TTF_Font* font, SynthState& state) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(renderer);

    // Afficher le nom de la forme d'onde
    std::string waveformStr;
    switch (*state.waveform) {
        case Waveform::Saw:       waveformStr = "Saw"; break;
        case Waveform::Square:    waveformStr = "Square"; break;
        case Waveform::Triangle:  waveformStr = "Triangle"; break;
        case Waveform::Noise:     waveformStr = "Noise"; break;
        case Waveform::Pulse:     waveformStr = "Pulse"; break;
        case Waveform::RampDown:  waveformStr = "RampDown"; break;
        case Waveform::SubOsc:    waveformStr = "SubOsc"; break;
        case Waveform::FilteredNoise: waveformStr = "FiltNoise"; break;
        case Waveform::DoubleSaw: waveformStr = "DoubleSaw"; break;
    }

    std::ostringstream oss;
    oss << "Waveform: " << waveformStr;
    renderText(renderer, font, oss.str(), 10, 10);
    
    // // Afficher le groupe de contrôle actif
    // std::ostringstream groupStr;
    // groupStr << "Control Group: " << (state.oscControlGroup == 0 ? "OSC Mix" : "ADSR/LFO");
    // renderText(renderer, font, groupStr.str(), 200, 10);
    
    int offsetX = 0;
    int offsetY = 0;

    // Afficher la fréquence active
    for (auto& voice : state.voices) {
        if (voice.active) {
            for (int i = 0; i < 6; ++i) {
                renderWaveformPeriod(renderer, state.renderBuffers[i], SynthState::WaveformBufferSize,
                                    state.renderBufferIndices[i], 470 + offsetX, 400 + offsetY, 400, 300, verticalScale, i);
                offsetX += 10;  // Décale chaque voix en X
                offsetY -= 10;  // Décale chaque voix en Y
            }

            std::ostringstream noteStr;
            noteStr << "Freq: " << voice.freq << " Hz";
            renderText(renderer, font, noteStr.str(), 10, 40);

            // Affichage de la jauge de filtre
            float normCutoff = voice.moogFilter.getCutoff() / 20000.0f;
            std::ostringstream cutoffStr;
            cutoffStr << "Cutoff:       " << createGauge(normCutoff, 20);
            renderText(renderer, font, cutoffStr.str(), 10, 320);

            // Jauge pour résonance
            float normResonance = voice.moogFilter.getResonance() / 4.0f;
            std::ostringstream resonanceStr;
            resonanceStr << "Resonance:    " << createGauge(normResonance, 20);
            renderText(renderer, font, resonanceStr.str(), 10, 350);

            // Jauge pour lfoDepth
            float normlfoDepth = state.lfoGlobalDepth;
            std::ostringstream lfoDepthStr;
            lfoDepthStr << "lfoDepth:     " << createGauge(normlfoDepth, 20);
            renderText(renderer, font, lfoDepthStr.str(), 10, 380);

            // Jauge pour lfoFrequency
            float normlfoFrequency = state.lfoFrequency / 20.0f;
            std::ostringstream lfoFrequencyStr;
            lfoFrequencyStr << "lfoFrequency: " << createGauge(normlfoFrequency, 20);
            renderText(renderer, font, lfoFrequencyStr.str(), 10, 410);

            renderWaveform(renderer, state.renderBuffer, SynthState::WaveformBufferSize, state.renderBufferIndex, 10, 150, 660, 200, verticalScale);
            
            int x = 690;
            int xLeft = x;
            int xRight = xLeft + 110;
            int xMiddle = 20;
            int lineHeight = 25;
            int baseY = 300;

            // === Affichage des modulations LFO ===
            int lfoX = x;
            int lfoY = baseY - lineHeight * 5;

            renderText(renderer, font, "LFO Cut", lfoX, lfoY);
            renderText(renderer, font, createFramedGaugeWithLevel(state.lfoDepthCutoff), lfoX + 30, lfoY);

            renderText(renderer, font, "LFO Pitch", lfoX, lfoY + lineHeight);
            renderText(renderer, font, createFramedGaugeWithLevel(state.lfoDepthPitch), lfoX + 30, lfoY + lineHeight);

            renderText(renderer, font, "LFO Pulse", xRight, lfoY + lineHeight);
            renderText(renderer, font, createFramedGaugeWithLevel(state.pulseWidthDepth), xRight + 30, lfoY + lineHeight);

            // === Affichage des réglages OSC (Groupe 0) ===
            renderText(renderer, font, "Saw", xLeft, baseY + lineHeight);
            renderText(renderer, font, "DSaw", xRight, baseY + lineHeight);
            renderText(renderer, font, createFramedGaugeWithLevel(state.mixSaw), xLeft + 30, baseY + lineHeight);
            renderText(renderer, font, createFramedGaugeWithLevel(state.mixDoubleSaw), xRight + 30, baseY + lineHeight);

            renderText(renderer, font, "Tri", xLeft, baseY);
            renderText(renderer, font, "Sq", xRight, baseY);
            renderText(renderer, font, createFramedGaugeWithLevel(state.mixTriangle), xLeft + 30, baseY);
            renderText(renderer, font, createFramedGaugeWithLevel(state.mixSquare), xRight + 30, baseY);

            renderText(renderer, font, "Pulse", xLeft, baseY - lineHeight);
            renderText(renderer, font, "Ramp", xRight, baseY - lineHeight);
            renderText(renderer, font, createFramedGaugeWithLevel(state.mixPulse), xLeft + 30, baseY - lineHeight);
            renderText(renderer, font, createFramedGaugeWithLevel(state.mixRampDown), xRight + 30, baseY - lineHeight);

            renderText(renderer, font, "Sub", xLeft, baseY - lineHeight * 2);
            renderText(renderer, font, "Noise", xRight, baseY - lineHeight * 2);
            renderText(renderer, font, createFramedGaugeWithLevel(state.mixSub), xLeft + 30, baseY - lineHeight * 2);
            renderText(renderer, font, createFramedGaugeWithLevel(state.mixNoise), xRight + 30, baseY - lineHeight * 2);

            // === CADRES DYNAMIQUES selon le groupe actif ===
            SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255); // Cadre bleu/violet

            if (state.oscControlGroup == 0) {
                // Cadre autour des réglages OSC (groupe 0)
                SDL_Rect oscMixRect;
                oscMixRect.x = xLeft - 10;
                oscMixRect.y = baseY - lineHeight * 2 - 10;
                oscMixRect.w = (xRight + 110) - oscMixRect.x;
                oscMixRect.h = lineHeight * 4 + 20;
                SDL_RenderDrawRect(renderer, &oscMixRect);
                
                // Titre du groupe actif
                renderText(renderer, font, "[OSC MIX]", xLeft, baseY - lineHeight * 3);
            } else if (state.oscControlGroup == 1) {
                // Cadre autour des réglages ADSR/LFO (groupe 1)
                SDL_Rect adsrLfoRect;
                adsrLfoRect.x = lfoX - 10;
                adsrLfoRect.y = lfoY - 10;
                adsrLfoRect.w = (xRight + 110) - adsrLfoRect.x;
                adsrLfoRect.h = lineHeight * 2 + 20;
                SDL_RenderDrawRect(renderer, &adsrLfoRect);
                
                // Cadre pour les réglages ADSR (dans la zone ADSR existante)
                SDL_Rect adsrRect;
                adsrRect.x = 785;  // Position des réglages ADSR
                adsrRect.y = 105;
                adsrRect.w = 85;   // Largeur pour englober A,D,S,R
                adsrRect.h = 50;
                SDL_RenderDrawRect(renderer, &adsrRect);
                
                // Titre du groupe actif
                renderText(renderer, font, "[ADSR/LFO]", lfoX, lfoY - lineHeight);
            }

            break;
        }
    }

    // Volume
    std::ostringstream volStr;
    volStr << "Volume: " << static_cast<int>(state.volume * 100) << "%";
    renderText(renderer, font, volStr.str(), 790, 70);
    renderText(renderer, font, createVerticalBlockGauge(state.volume), 790, 40);

    renderText(renderer, font, "Press ESC to quit", 10, 110);
    renderText(renderer, font, "CC115/116: Switch Groups", 10, 70);
    
    renderADSR(renderer, font, state, 300, 20, 400, 100);

    SDL_RenderPresent(renderer);
}

