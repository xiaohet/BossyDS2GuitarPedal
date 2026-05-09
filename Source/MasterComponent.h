/*
  ==============================================================================

    MasterComponent.h
    Author:  Xiaohe Tian

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CustomComponent.h"

//==============================================================================
/*
*/
class MasterComponent : public CustomComponent
{
public:
    MasterComponent(juce::AudioProcessorValueTreeState& apvts, juce::String masterBypass, juce::String toneId, juce::String distId);
    ~MasterComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    SliderWithLabel tone;
    SliderWithLabel dist;

    juce::ToggleButton bypass;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    static constexpr int hSliderWidth = 200;
    static constexpr int hSliderHeight = 30;
    static constexpr int vSliderWidth = 50;
    static constexpr int vSliderHeight = 60;
    static constexpr int dialWidth = 80;
    static constexpr int dialHeight = 80;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MasterComponent)
};
