#include "ADSR.h"
#include <algorithm>  // pour std::clamp dans les versions récentes de C++

ADSR::ADSR(float sampleRate)
    : sampleRate(sampleRate), state(IDLE), value(0.0f),
      attackTime(0.05f), decayTime(0.9f), sustainLevel(0.9f), releaseTime(0.9f) {
    calculateRates();
}

void ADSR::calculateRates() {
    // Assurer des valeurs minimales pour éviter les divisions par zéro
    float safeAttackTime = std::max(0.001f, attackTime);
    float safeDecayTime = std::max(0.001f, decayTime);
    float safeReleaseTime = std::max(0.001f, releaseTime);
    
    attackRate = 1.0f / (safeAttackTime * sampleRate);
    decayRate = (1.0f - sustainLevel) / (safeDecayTime * sampleRate);
    releaseRate = sustainLevel / (safeReleaseTime * sampleRate);

    float safeAntiClickTime = std::max(0.0001f, antiClickTime);  // 100 µs minimum
    antiClickRate = antiClickTarget / (safeAntiClickTime * sampleRate);
}

void ADSR::setAttack(float value) {
    attackTime = value;  // Déjà en secondes
    calculateRates();
}

void ADSR::setDecay(float value) {
    decayTime = value;  // Déjà en secondes
    calculateRates();
}

void ADSR::setSustain(float value) {
    sustainLevel = std::min(1.0f, std::max(0.0f, value)); // Limiter entre 0 et 1
    calculateRates();
}

void ADSR::setRelease(float value) {
    releaseTime = value;  // Déjà en secondes
    calculateRates();
}

void ADSR::setADSR(float attack, float decay, float sustain, float release) {
    setAttack(attack);
    setDecay(decay);
    setSustain(sustain);
    setRelease(release);
}

void ADSR::noteOn() {
    // CORRECTION PRINCIPALE : Ne pas remettre value à 0 brutalement
    // Recalculer attackRate en fonction de la valeur actuelle
    
    if (value > 0.0f && state != IDLE) {
        // Si on est déjà en train de jouer une note, ajuster l'attack
        float remainingAttack = (1.0f - value);
        float adjustedAttackTime = attackTime * remainingAttack;
        float safeAdjustedAttackTime = std::max(0.001f, adjustedAttackTime);
        attackRate = remainingAttack / (safeAdjustedAttackTime * sampleRate);
    } else {
        // Premier noteOn ou note complètement finie
        value = 0.0f;
        calculateRates(); // Utiliser l'attackRate normal
    }
    
    state = ATTACK;
}

void ADSR::noteOff() {
    // S'assurer qu'on ne part pas de zéro pour le release
    if (value <= 0.0f) {
        value = 0.001f;
    }
    
    // Recalculer le release rate en fonction de la valeur actuelle
    float safeReleaseTime = std::max(0.001f, releaseTime);
    releaseRate = value / (safeReleaseTime * sampleRate);
    
    state = RELEASE;
}

float ADSR::process() {
    switch (state) {
        case ATTACK:
            value += attackRate;
            if (value >= 1.0f) {
                value = 1.0f;
                state = DECAY;
                // Recalculer le decay rate au cas où sustainLevel aurait changé
                float safeDecayTime = std::max(0.001f, decayTime);
                decayRate = (1.0f - sustainLevel) / (safeDecayTime * sampleRate);
            }
            break;
            
        case DECAY:
            value -= decayRate;
            if (value <= sustainLevel) {
                value = sustainLevel;
                state = SUSTAIN;
            }
            break;
            
        case SUSTAIN:
            // Maintenir le niveau de sustain
            value = sustainLevel;
            break;
            
        case RELEASE:
            value -= releaseRate;
            if (value <= 0.0f) {
                value = 0.0f;
                state = IDLE;
            }
            break;
            
        case IDLE:
            value = 0.0f;
            break;
    }
    
    return value;
}

bool ADSR::isActive() const {
    return state != IDLE;
}

void ADSR::reset() {
    state = IDLE;
    value = 0.0f;
    calculateRates(); // Recalcule les rates avec les valeurs par défaut
}