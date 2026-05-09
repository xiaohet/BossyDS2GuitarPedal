#include "DSPPedal.h"
#include <fstream>

void DSPPedal::prepare(double sampleRate, int samplesPerBlock, int numChannels)
{
    // read waveshaper lookup table
    lut.resize(lutSizeD * lutSizeX);

    memcpy(
        lut.data(),
        BinaryData::waveshaper_2d_bin,
        std::min((size_t)BinaryData::waveshaper_2d_binSize,
            lut.size() * sizeof(float))
    );

    // initialize filters
    darkFilters.resize(numChannels);
    brightFilters.resize(numChannels);

    float b0, b1, b2, a1, a2;

    for (int i = 0; i < numChannels; i++) {
        darkFilters[i].reset();
        brightFilters[i].reset();

        makeLowpass(sampleRate, 2000.0f, 0.707f, b0, b1, b2, a1, a2);
        darkFilters[i].setCoefficients(b0, b1, b2, a1, a2);
        DBG("init dark filters: b0: " << b0 << " a1: " << a1);
        makeHighpass(sampleRate, 800.0f, 0.707f, b0, b1, b2, a1, a2);
        brightFilters[i].setCoefficients(b0, b1, b2, a1, a2);
        DBG("init bright filters: b0: " << b0 << " a1: " << a1);
    }


    // initialize oversampling
    oversampling = std::make_unique<juce::dsp::Oversampling<float>>(
        2,  // number of channels
        3,  // oversampling stages (2^3 = 8)
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR
    );
    oversampling->initProcessing(samplesPerBlock);

}

float cubicInterp(float y0, float y1, float y2, float y3, float t)
{
    float a0 = -0.5f * y0 + 1.5f * y1 - 1.5f * y2 + 0.5f * y3;
    float a1 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
    float a2 = -0.5f * y0 + 0.5f * y2;
    float a3 = y1;

    return ((a0 * t + a1) * t + a2) * t + a3;
}

float DSPPedal::processWaveshaper(float x)
{
    static int counter = 0;
 
    // clamp input
    x = juce::jlimit(-5.0f, 5.0f, x);

    // normalize x → index
    float xNorm = (x + 5.0f) / 10.0f;  // [-5,5] → [0,1]
    float xIndex = xNorm * (lutSizeX - 1);

    int x0 = (int)xIndex;
    int x1 = std::min(x0 + 1, lutSizeX - 1);
    float fracX = xIndex - x0;

    // normalize distortion → index
    float d = juce::jlimit(0.0f, 1.0f, dist);
    float dIndex = d * (lutSizeD - 1);

    int d0 = (int)dIndex;
    int d1 = std::min(d0 + 1, lutSizeD - 1);
    float fracD = dIndex - d0;

    // helper: 2D → 1D index
    auto idx = [this](int d, int x)
        {
            return d * lutSizeX + x;
        };

    auto idx00 = idx(d0, x0);
    auto idx01 = idx(d0, x1);
    auto idx10 = idx(d1, x0);
    auto idx11 = idx(d1, x1);


    // fetch 4 points
    float v00 = lut[idx(d0, x0)];
    float v01 = lut[idx(d0, x1)];
    float v10 = lut[idx(d1, x0)];
    float v11 = lut[idx(d1, x1)];

    int i1 = x0;
    int i2 = std::min(x0 + 1, lutSizeX - 1);
    int i0m = std::max(i1 - 1, 0);
    int i3 = std::min(i2 + 1, lutSizeX - 1);

    float v0 = cubicInterp(
        lut[d0 * lutSizeX + i0m],
        lut[d0 * lutSizeX + i1],
        lut[d0 * lutSizeX + i2],
        lut[d0 * lutSizeX + i3],
        fracX
    );

    float v1 = cubicInterp(
        lut[d1 * lutSizeX + i0m],
        lut[d1 * lutSizeX + i1],
        lut[d1 * lutSizeX + i2],
        lut[d1 * lutSizeX + i3],
        fracX
    );

    float v = v0 + fracD * (v1 - v0);

    return v;
}

float DSPPedal::processSample(float x)
{
    static int counter = 0;

    float preGain = 1.0f + 8.0f * dist; // 20.0f
    float driven = x * preGain;

    float shaped = processWaveshaper(driven);

    float xDist = dist * shaped + (1.0f - dist) * x;

    float yDark = darkFilters[0].process(x);
    float yBright = brightFilters[0].process(x);

    jassert(!std::isnan(xDist));
    jassert(std::abs(yDark) < 100.0f);

    return tone * yBright + (1.0f - tone) * yDark;
}

void DSPPedal::processBlock(juce::AudioBuffer<float>& buffer) {

    static int counter = 0;

    juce::dsp::AudioBlock<float> block(buffer);
    auto oversampledBlock = oversampling->processSamplesUp(block);
    int numSamples = oversampledBlock.getNumSamples();

    float alpha = 0.001f; // smoothing

    for (size_t ch = 0; ch < oversampledBlock.getNumChannels(); ++ch)
    {
        auto* data = oversampledBlock.getChannelPointer(ch);

        for (int i = 0; i < numSamples; ++i)
        {
            float x = data[i];

            envelopePerChannel[ch] = (1.0f - alpha) * envelopePerChannel[ch] + alpha * std::abs(x);
            //float norm = x / (0.1f + envelopePerChannel[ch]);
            //norm = juce::jlimit(-5.0f, 5.0f, norm);

            float preGain = 1.0f + 20.0f * dist; // 10.0f
            //float targetLevel = 1.0f;
            //float driven = norm * targetLevel * preGain;
            float driven = x * preGain;

            float shaped = processWaveshaper(driven);

            float y = shaped; // dist* shaped + (1.0f - dist) * x;

            float gainComp;
            
            gainComp = 1.0f / (1.0f + 8.0f * dist * (1.0f - dist)) * (1.0f + 3.0f * (1.0f - dist));
            y *= gainComp;
            data[i] = y;

            //if (++counter == 10000)
            //{
            //    int dist_idx = (int)(dist * lutSizeD);
            //    DBG("envelope(ch " << ch << "): " << envelopePerChannel[ch] << " dist: " << dist << " x: " << x);
            //    counter = 0;
            //}
        }
    }

    oversampling->processSamplesDown(block);

    // now apply tone filters at normal sample rate
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer(ch);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float x = data[i];

            //if (std::abs(x) < 1e-8f)
            //    x = 0.0f;

            //if (std::abs(x) < 1e-5f)
            //{
            //    darkFilters[ch].reset();
            //    brightFilters[ch].reset();
            //}

            float yDark = darkFilters[ch].process(x);
            float yBright = brightFilters[ch].process(x);

            data[i] = tone * yBright + (1.0f - tone) * yDark;


            if (++counter == 1000)
            {
                DBG("tone: " << tone);
                counter = 0;
            }

        }

    }

}

void DSPPedal::setDist(float newDist) {
    dist = newDist / 100.0; // percent to direct value
}

void DSPPedal::setTone(float newTone) {
    tone = newTone / 100.0; // percent to direct value
}
