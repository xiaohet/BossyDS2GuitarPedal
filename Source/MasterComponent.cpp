/*
  ==============================================================================

    MasterComponent.cpp
    Author:  Xiaohe Tian

  ==============================================================================
*/

#include <JuceHeader.h>
#include "MasterComponent.h"

//==============================================================================
MasterComponent::MasterComponent(juce::AudioProcessorValueTreeState& apvts, juce::String masterBypass, juce::String toneId, juce::String distId)
    : tone("Tone", toneId, apvts, dialWidth, dialHeight, "%")
    , dist("Distortion", distId, apvts, dialWidth, dialHeight, "%")
{
    addAndMakeVisible(tone);
    addAndMakeVisible(dist);

    bypass.setButtonText("Bypass");
    addAndMakeVisible(bypass);
    bypassAttachment = (std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>)
        new  juce::AudioProcessorValueTreeState::ButtonAttachment(apvts, masterBypass, bypass);

    bool bypassState = apvts.getRawParameterValue(masterBypass);
    //enable.setToggleState(enableState, juce::NotificationType::dontSendNotification);
    if (bypassState == true)
    {
        DBG("bypass init on");
    }
    else
    {
        DBG("bypass init off");
    }
    bypass.onClick = [this]() {
        if (bypass.getToggleState())
        {
            DBG("bypass click on");
        }
        else
        {
            DBG("bypass click off");
        }
    };
}

MasterComponent::~MasterComponent()
{
}

void MasterComponent::resized()
{
    const auto xStart = 20;
    const auto yStart = 15;
    const auto vWidth = vSliderWidth;
    const auto vHeight = vSliderHeight + 20;
    const auto width = dialWidth + 20;
    const auto height = dialHeight + 20;

    tone.setBounds(getWidth() / 2 - 60 - 40, 50 + (getHeight() - 100) / 2 - 60, width, height);
    dist.setBounds(getWidth() / 2 + 60 - 40, 50 + (getHeight() - 100) / 2 - 60, width, height);
    bypass.setBounds(tone.getX(), tone.getBottom(), 80, 40);
}

void MasterComponent::paint(juce::Graphics& g)
{
    // g.fillAll (juce::Colours::black);
    auto bounds = getLocalBounds();
    g.setColour(juce::Colours::white);
    g.drawRoundedRectangle(bounds.toFloat().reduced(3.0f), 3.0f, 1.0f);

    g.setColour(juce::Colours::whitesmoke);
    g.setFont(fontHeight);
    g.setFont(g.getCurrentFont().boldened());

    // g.drawText("Pedals", 15, 5, getWidth()-30, 20, juce::Justification::centredTop);

}

