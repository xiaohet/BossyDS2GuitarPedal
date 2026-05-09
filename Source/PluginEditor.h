/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "MasterComponent.h"

//==============================================================================
/**
*/
class PedalModelAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    PedalModelAudioProcessorEditor (PedalModelAudioProcessor&);
    ~PedalModelAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PedalModelAudioProcessor& audioProcessor;

    MasterComponent pedals;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PedalModelAudioProcessorEditor)
};
