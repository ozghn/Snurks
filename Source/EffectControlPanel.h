/*
  ==============================================================================
    EffectControlPanel.h
  ==============================================================================
*/
#pragma once

#include <JuceHeader.h>

enum class EffectGroup
{
    BitcrusherDrive,
    GlitchDropout,
    Granular,
    PitchStereo
};

class EffectControlPanel : public juce::Component
{
public:
    EffectControlPanel(juce::AudioProcessorValueTreeState&);

    void setEffectGroup(EffectGroup group);
    void resized() override;

private:
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    void configureForGroup(EffectGroup group);
    void clearAttachments();

    juce::AudioProcessorValueTreeState& apvts;

    std::array<juce::Slider, 4> sliders;
    std::array<juce::Label, 4>  labels;
    std::array<std::unique_ptr<Attachment>, 4> attachments;

    EffectGroup currentGroup{ EffectGroup::BitcrusherDrive };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectControlPanel)
};