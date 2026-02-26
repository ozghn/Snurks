/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "SnurksLNF.h"
#include "EffectControlPanel.h"

class Snurks_V1AudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    Snurks_V1AudioProcessorEditor(Snurks_V1AudioProcessor&);
    ~Snurks_V1AudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    Snurks_V1AudioProcessor& audioProcessor;
    juce::SnurksLNF lnf;
    juce::Image snurkBackground;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    // === Global Macro Controls ===
    juce::Slider fadeInSlider;
    juce::Label  fadeLabel;

    juce::Slider rampSpeedSlider;
    juce::Label  rampLabel;

    juce::ToggleButton enableButton;
    juce::Label        enableLabel;

    // === Effect Selection & Parameters ===
    juce::ComboBox     effectSelector;
    EffectControlPanel effectPanel;

    // === APVTS Attachments ===
    std::unique_ptr<SliderAttachment> fadeInAttachment;
    std::unique_ptr<SliderAttachment> rampSpeedAttachment;
    std::unique_ptr<ButtonAttachment> enableAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Snurks_V1AudioProcessorEditor)
};
