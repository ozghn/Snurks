/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Snurks_V1AudioProcessor::Snurks_V1AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    
}

Snurks_V1AudioProcessor::~Snurks_V1AudioProcessor()
{
}

//==============================================================================
const juce::String Snurks_V1AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Snurks_V1AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Snurks_V1AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Snurks_V1AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Snurks_V1AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Snurks_V1AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Snurks_V1AudioProcessor::getCurrentProgram()
{
    return 0;
}

void Snurks_V1AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Snurks_V1AudioProcessor::getProgramName (int index)
{
    return {};
}

void Snurks_V1AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Snurks_V1AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
	snurks.prepare(sampleRate, samplesPerBlock);
}

void Snurks_V1AudioProcessor::releaseResources()
{
 
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Snurks_V1AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else

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

void Snurks_V1AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());


	// Update parameters from the AudioProcessorValueTreeState
	auto& bitDepth = *apvts.getRawParameterValue("bitDepth");
	auto& downsampleFactor = *apvts.getRawParameterValue("downsampleFactor");

	auto& glitchLength = *apvts.getRawParameterValue("glitchLength");
	auto& glitchChance = *apvts.getRawParameterValue("glitchChance");

	auto& grainSize = *apvts.getRawParameterValue("grainSize");
	auto& grainSpacing = *apvts.getRawParameterValue("grainSpacing");
	auto& playbackRate = *apvts.getRawParameterValue("playbackRate");
	auto& jitterAmount = *apvts.getRawParameterValue("jitterAmount");

	auto& pitchDepth = *apvts.getRawParameterValue("pitchDepth");
	auto& pitchRate = *apvts.getRawParameterValue("pitchRate");

	auto& drive = *apvts.getRawParameterValue("drive");
	auto& driveBlend = *apvts.getRawParameterValue("driveBlend");

	auto& stereoDepth = *apvts.getRawParameterValue("stereoDepth");
	auto& stereoRate = *apvts.getRawParameterValue("stereoRate");

	snurks.updateBitcrusher(bitDepth.load(), downsampleFactor.load());
	snurks.updateGlitch(glitchLength.load(), glitchChance.load());
	snurks.updateGranular(grainSize.load(), grainSpacing.load(), playbackRate.load(), jitterAmount.load());
	snurks.updatePitchmod(pitchDepth.load(), pitchRate.load());
	snurks.updateDistortion(drive.load(), driveBlend.load());
	snurks.updateStereoPanner(stereoDepth.load(), stereoRate.load());

    //===Macro Controls===
    auto& enableEffect = *apvts.getRawParameterValue("enableEffect");
	auto& fadeInDuration = *apvts.getRawParameterValue("fadeInDuration");
	auto& bitDepthRampSpeed = *apvts.getRawParameterValue("bitDepthRampSpeed");

	snurks.setFadeInDuration(fadeInDuration.load());
	snurks.setBitDepthRampSpeed(bitDepthRampSpeed.load());

    
	
    juce::AudioSourceChannelInfo info(buffer);
	
	

	// Check toggle and process the audio buffer
	bool snurking = enableEffect.load();
	snurks.triggerEffect(snurking);
    snurks.updateFadeAndBitcrusher(buffer.getNumSamples());

    snurks.updateFadeAndBitcrusher(buffer.getNumSamples());


	snurks.processBlock(buffer, buffer.getNumSamples());

}

//==============================================================================
bool Snurks_V1AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Snurks_V1AudioProcessor::createEditor()
{
    return new Snurks_V1AudioProcessorEditor (*this);
}

//==============================================================================
void Snurks_V1AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Snurks_V1AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Snurks_V1AudioProcessor();
}

// === PARAMTERS ===
juce::AudioProcessorValueTreeState::ParameterLayout Snurks_V1AudioProcessor::createParameterLayout()
{
	juce::AudioProcessorValueTreeState::ParameterLayout layout;
	
    layout.add(std::make_unique<juce::AudioParameterFloat>("bitDepth", "Bit Depth", 4.0f, 16.0f, 8.0f));
	layout.add(std::make_unique<juce::AudioParameterInt>("downsampleFactor", "Downsample Factor", 1, 20, 1));

	layout.add(std::make_unique<juce::AudioParameterFloat>("glitchLength", "Glitch Length", 64.0f, 2048.0f, 512.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("glitchChance", "Glitch Chance", juce::NormalisableRange<float>(0.0001f, 1.0f, 0.0001f, 0.3f),
        0.005f));

	layout.add(std::make_unique<juce::AudioParameterFloat>("grainSize", "Grain Size", 10.0f, 200.0f, 60.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("grainSpacing", "Grain Spacing", 1.0f, 100.0f, 20.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("playbackRate", "Playback Rate", juce::NormalisableRange<float>(0.25f, 2.0f, 0.01f), 1.0f));
	layout.add(std::make_unique<juce::AudioParameterInt>("jitterAmount", "Jitter Amount", 0, 20, 0));

    layout.add(std::make_unique<juce::AudioParameterFloat>("pitchDepth", "Pitch Depth", juce::NormalisableRange<float>(0.1f, 50.0f, 0.01f), 8.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>( "pitchRate", "Pitch Rate",juce::NormalisableRange<float>(0.01f, 10.0f, 0.01f), 5.0f));
	
	layout.add(std::make_unique<juce::AudioParameterFloat>("drive", "Drive", 0.0f, 1.0f, 0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("driveBlend", "Blend", 0.0f, 1.0f, 0.5f));

	layout.add(std::make_unique<juce::AudioParameterFloat>("stereoDepth", "Stereo Depth", 0.0f, 1.0f, 0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("stereoRate", "Stereo Rate", 0.01f, 10.0f, 1.0f));

	layout.add(std::make_unique<juce::AudioParameterFloat>("dropoutRate", "Dropout Rate", 0.0f, 1.0f, 0.5f));
	layout.add(std::make_unique<juce::AudioParameterInt>("holdDuration", "Hold Duration", 1, 44100, 200));

	layout.add(std::make_unique<juce::AudioParameterFloat>("fadeInDuration", "Fade In Duration", 0.01f, 10.0f, 0.1f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("bitDepthRampSpeed", "Bit Depth Ramp Speed", 0.01f, 10.0f, 0.1f));
	layout.add(std::make_unique<juce::AudioParameterBool>("enableEffect", "Enable Effect", false));


    return layout;
}
