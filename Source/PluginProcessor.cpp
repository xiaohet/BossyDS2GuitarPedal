/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PedalModelAudioProcessor::PedalModelAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
    , parameters(*this, nullptr, juce::Identifier("ProcessorChain"),
    {std::make_unique<juce::AudioParameterInt>(juce::ParameterID("Tone", 1), "Tone", 0, 100, 50),
    std::make_unique<juce::AudioParameterInt>(juce::ParameterID("Distortion", 1), "Distortion", 0, 100, 50),
     std::make_unique<juce::AudioParameterBool>(juce::ParameterID("Bypass", 1), "Bypass", false),
        })
#endif
{
    // file writing
    juce::File file = juce::File::getSpecialLocation(
        juce::File::userDesktopDirectory
    ).getChildFile("debug.raw");
    debugFile.open(file.getFullPathName().toStdString(), std::ios::binary | std::ios::out | std::ios::trunc);
    if (!debugFile.is_open())
        DBG("Failed to open debug file!");
    else
        DBG("Debug file path: " << file.getFullPathName());
}

PedalModelAudioProcessor::~PedalModelAudioProcessor()
{
}

//==============================================================================
const juce::String PedalModelAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PedalModelAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PedalModelAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PedalModelAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PedalModelAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PedalModelAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PedalModelAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PedalModelAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PedalModelAudioProcessor::getProgramName (int index)
{
    return {};
}

void PedalModelAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PedalModelAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    // 
    //juce::dsp::ProcessSpec spec;
    //spec.sampleRate = sampleRate;
    //spec.maximumBlockSize = samplesPerBlock;
    //spec.numChannels = 2;
    int numChannels = getTotalNumOutputChannels();

    pedal.prepare(sampleRate, samplesPerBlock, numChannels);
}

void PedalModelAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PedalModelAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void PedalModelAudioProcessor::setParams() {
    auto& tone = *parameters.getRawParameterValue("Tone");
    auto& dist = *parameters.getRawParameterValue("Distortion");

    pedal.setDist(dist);
    pedal.setTone(tone);

}


// For real-time audio capturing and dumping.
void PedalModelAudioProcessor::liveAudioDump(juce::AudioBuffer<float>& buffer) {
    if (!debugFile.is_open())
    {
        DBG("File NOT open in processBlock!");
    }
    else
    {
        debugFile.write(reinterpret_cast<const char*>(buffer.getReadPointer(0)),
            buffer.getNumSamples() * sizeof(float));
        DBG("buffer written, size: " << buffer.getNumSamples() * sizeof(float));

        debugFile.flush();
        debugFile.clear();
    }
}

void PedalModelAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    int numSamples = buffer.getNumSamples();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    setParams();
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.

    bool isBypassed = *parameters.getRawParameterValue("Bypass");
    if (isBypassed)
    {
        liveAudioDump(buffer);
        return; // pass-through
    }

    pedal.processBlock(buffer);

    liveAudioDump(buffer);

    //for (int channel = 0; channel < totalNumInputChannels; ++channel)
    //{
    //    auto* channelData = buffer.getWritePointer (channel);

    //    for (int i = 0; i < numSamples; ++i)
    //        channelData[i] = pedal.processSample(channelData[i]);
    //}
}

//==============================================================================
bool PedalModelAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PedalModelAudioProcessor::createEditor()
{
    return new PedalModelAudioProcessorEditor (*this);
}

//==============================================================================
void PedalModelAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void PedalModelAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PedalModelAudioProcessor();
}
