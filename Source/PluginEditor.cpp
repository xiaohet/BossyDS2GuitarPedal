/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PedalModelAudioProcessorEditor::PedalModelAudioProcessorEditor (PedalModelAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
    , pedals(audioProcessor.parameters, "Bypass", "Tone", "Distortion")
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (300, 250);

    addAndMakeVisible(pedals);

    pedals.setName("Pedals");
}

PedalModelAudioProcessorEditor::~PedalModelAudioProcessorEditor()
{
}

//==============================================================================
void PedalModelAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colour::fromRGB(228, 228, 64));
    g.setFont (juce::FontOptions ("Forte", 30.0f, juce::Font::plain));
    g.drawFittedText ("Bossy DS-2", 0, 10, getWidth(), 40, juce::Justification::centredTop, 1);
}

void PedalModelAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    pedals.setBounds(20, 50, getWidth() - 40, getHeight() - 70);
}
