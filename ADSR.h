#pragma once

class ADSR {
public:
    ADSR(float sampleRate);

    void noteOn();
    void noteOff();
    float process();
    bool isActive() const;

    void reset();
    
    float getAttack() const { return attackTime; }
    void setAttack(float a);
    float getDecay() const { return decayTime; }
    void setDecay(float d);
    float getSustain() const { return sustainLevel; }
    void setSustain(float s);
    float getRelease() const { return releaseTime; }
    void setRelease(float r);
    float getSampleRate() const { return sampleRate; }

    void setADSR(float attack, float decay, float sustain, float release);

private:
    enum State { ATTACK, DECAY, SUSTAIN, RELEASE, IDLE };
    // enum State { ANTI_CLICK_IN, ATTACK, DECAY, SUSTAIN, RELEASE, IDLE };

    State state;
    float value;
    float sampleRate;
    float attackTime, decayTime, sustainLevel, releaseTime;
    float attackRate, decayRate, releaseRate;

    float antiClickTime = 0.002f; // 2ms par défaut
    float antiClickRate;
    float antiClickTarget;

    void calculateRates();
};