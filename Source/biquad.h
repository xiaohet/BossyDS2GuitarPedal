#pragma once
#include <JuceHeader.h>

class Biquad
{
public:

    void setCoefficients(float b0, float b1, float b2, float a1, float a2)
    {
        this->b0 = b0;
        this->b1 = b1;
        this->b2 = b2;
        this->a1 = a1;
        this->a2 = a2;
    }

    void reset()
    {
        x1 = x2 = y1 = y2 = 0.0f;
    }

    float process(float x)
    {
        // Direct Form I
        float y = b0 * x + b1 * x1 + b2 * x2
            - a1 * y1 - a2 * y2;

        // shift states
        x2 = x1;
        x1 = x;
        y2 = y1;
        y1 = y;

        // denormal protection

        if (std::abs(x1) < 1e-10f) x1 = 0.0f;
        if (std::abs(x2) < 1e-10f) x2 = 0.0f;
        if (std::abs(y1) < 1e-10f) y1 = 0.0f;
        if (std::abs(y2) < 1e-10f) y2 = 0.0f;
        // if (std::abs(y) < 1e-10f) y = 0.0f;

        return y;
    }


private:
    float b0 = 1, b1 = 0, b2 = 0;
    float a1 = 0, a2 = 0;

    float x1 = 0, x2 = 0;
    float y1 = 0, y2 = 0;
};