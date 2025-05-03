#pragma once

class ADSR {
public:
    ADSR(float sampleRate);

    void noteOn();
    void noteOff();
    float process();
    bool isActive() const;
    
    float getAttack() const { return attackTime; }
    void setAttack(float a);
    float getDecay() const { return decayTime; }
    void setDecay(float d);
    float getSustain() const { return sustainLevel; }
    void setSustain(float s);
    float getRelease() const { return releaseTime; }
    void setRelease(float r);
    float getSampleRate() const { return sampleRate; }

private:
    enum State { ATTACK, DECAY, SUSTAIN, RELEASE, IDLE };
    State state;
    float value;
    float sampleRate;
    float attackTime, decayTime, sustainLevel, releaseTime;
    float attackRate, decayRate, releaseRate;
    
    void calculateRates();
};