#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include <JuceHeader.h>
#include "biquad.h"

class DSPPedal
{
public:
    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    float processSample(float x);
    void processBlock(juce::AudioBuffer<float>& buffer);

    void setDist(float newDist);
    void setTone(float newTone);

    void makeLowpass(float fs, float fc, float Q,
        float& b0, float& b1, float& b2,
        float& a1, float& a2)
    {
        float w0 = 2.0f * float(M_PI) * fc / fs;
        float cosw0 = std::cos(w0);
        float sinw0 = std::sin(w0);
        float alpha = sinw0 / (2.0f * Q);

        float a0 = 1.0f + alpha;

        b0 = (1.0f - cosw0) / 2.0f / a0;
        b1 = (1.0f - cosw0) / a0;
        b2 = (1.0f - cosw0) / 2.0f / a0;
        a1 = -2.0f * cosw0 / a0;
        a2 = (1.0f - alpha) / a0;
    }

    void makeHighpass(float fs, float fc, float Q,
        float& b0, float& b1, float& b2,
        float& a1, float& a2)
    {
        float w0 = 2.0f * float(M_PI) * fc / fs;
        float cosw0 = std::cos(w0);
        float sinw0 = std::sin(w0);
        float alpha = sinw0 / (2.0f * Q);

        float a0 = 1.0f + alpha;

        b0 = (1.0f + cosw0) / 2.0f / a0;
        b1 = -(1.0f + cosw0) / a0;
        b2 = (1.0f + cosw0) / 2.0f / a0;
        a1 = -2.0f * cosw0 / a0;
        a2 = (1.0f - alpha) / a0;
    }
private:
    float processWaveshaper(float x);

    static constexpr int lutSizeX = 8192;
    static constexpr int lutSizeD = 64;

    std::vector<float> lut;

    float dist = 0.5f;
    float tone = 0.5f;

    std::vector<Biquad> darkFilters;
    std::vector<Biquad> brightFilters;

    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;

    std::vector<float> envelopePerChannel = { 0.0f, 0.0f };

};