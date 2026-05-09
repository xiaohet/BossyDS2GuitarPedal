/*
  ==============================================================================

    CustomComponent.cpp
    Author:  Xiaohe Tian
    In reference to "CustomComponent.cpp" by Joshua Hodge.

  ==============================================================================
*/

#include <JuceHeader.h>
#include "CustomComponent.h"

//==============================================================================

using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
using SliderStyle = juce::Slider::SliderStyle;

SliderWithLabel::SliderWithLabel(juce::String labelName, juce::String paramId, juce::AudioProcessorValueTreeState& apvts, const int width, const int height, juce::String unit, SliderStyle style)
{
    sliderWidth = width;
    sliderHeight = height;
    
    // set slider and knob styles and looks including label
    slider.setSliderStyle (style);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, width > 85 ? 80 : width - 5, 20);
    slider.setTextValueSuffix(unit);
    slider.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, juce::Colour::fromRGB(208, 234, 255));
    slider.setColour(juce::Slider::ColourIds::trackColourId, juce::Colour::fromRGB(208, 234, 255));
    slider.setColour(juce::Slider::ColourIds::thumbColourId, juce::Colour::fromRGB(160, 208, 255));
    addAndMakeVisible (slider);
    
    label.setFont (juce::FontOptions(fontHeight));
    label.setText (labelName, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (label);
    
    attachment = std::make_unique<SliderAttachment>(apvts,  paramId,  slider);
}

void SliderWithLabel::resized()
{
    const auto dialToLabelRatio = 15;
    const auto labelHeight = 18;
    
    jassert (sliderWidth > 0);
    jassert (sliderHeight > 0);
    
    label.setBounds(0, 0, sliderWidth, labelHeight);
    slider.setBounds(0, 0 + dialToLabelRatio, sliderWidth, sliderHeight);
}


CustomComponent::CustomComponent()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

CustomComponent::~CustomComponent()
{
}

void CustomComponent::paint (juce::Graphics& g)
{
    // g.fillAll (juce::Colours::black); // do not fill background color yet; use default background color
    auto bounds = getLocalBounds();
    g.setColour (boundsColour);
    g.drawRoundedRectangle (bounds.toFloat().reduced (3.0f), 3.0f, 1.0f);
    
    g.setColour (juce::Colours::whitesmoke);
    g.setFont (fontHeight);
    g.setFont (g.getCurrentFont().boldened());
    
    jassert (name.isNotEmpty());
    g.drawText (name, 15, 5, 100, 20, juce::Justification::left);
}

void CustomComponent::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

