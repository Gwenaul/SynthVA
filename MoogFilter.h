#pragma once
#include <cmath>

class MoogFilter {
    
public:
    MoogFilter(float sampleRate) : sampleRate(sampleRate) {
        setCutoff(1000.0f);
        setResonance(0.5f);
    }

    void setCutoff(float cutoffHz) {
        cutoff = cutoffHz;
        float f = (cutoffHz / sampleRate) * 1.16f;
        p = f * (1.8f - 0.8f * f);
        k = 2.0f * sinf(f * M_PI * 0.5f) - 1.0f;
    }

    void setResonance(float resonance) {

        if (resonance < 0.0f) resonance = 0.0f;
        if (resonance > 4.0f) resonance = 4.0f;

        res = resonance;
    }

    float process(float input) {
        input -= out4 * res;
        input *= 0.35013f * (p*p)*(p*p);

        out1 = input + 0.3f * in1 + (1 - p) * out1; 
        in1 = input;
        out2 = out1 + 0.3f * in2 + (1 - p) * out2;
        in2 = out1;
        out3 = out2 + 0.3f * in3 + (1 - p) * out3;
        in3 = out2;
        out4 = out3 + 0.3f * in4 + (1 - p) * out4;
        in4 = out3;

        return out4;
    }
    
public:
    float getCutoff() const { return cutoff; }
    float getResonance() const { return res; }

private:
    float sampleRate;
    float cutoff = 1000.0f;
    float res = 0.5f;

    float in1 = 0.0f, in2 = 0.0f, in3 = 0.0f, in4 = 0.0f;
    float out1 = 0.0f, out2 = 0.0f, out3 = 0.0f, out4 = 0.0f;

    float p = 0.0f, k = 0.0f;
};
