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

void ADSR::noteOn() {
    antiClickTarget = 0.001f;  // ou un niveau initial très faible
    value = 0.0f;
    // state = ANTI_CLICK_IN;
    state = ATTACK;
}


void ADSR::noteOff() {
    value = std::max(value, 0.001f);  // éviter release depuis zéro
    state = RELEASE;
}


float ADSR::process() {
    switch (state) {
        // case ANTI_CLICK_IN:
        //     value += antiClickRate;
        //     if (value >= antiClickTarget) {
        //         value = antiClickTarget;
        //         state = ATTACK;
        //     }
        //     break;
        case ATTACK:
            value += attackRate;
            if (value >= 1.0f) {
                value = 1.0f;
                state = DECAY;
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
            // Rien
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