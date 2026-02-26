/*
  ==============================================================================
    EffectControlPanel.cpp
  ==============================================================================
*/
#include "EffectControlPanel.h"

EffectControlPanel::EffectControlPanel(juce::AudioProcessorValueTreeState& state)
    : apvts(state)
{
    for (int i = 0; i < 4; ++i)
    {
        sliders[i].setSliderStyle(juce::Slider::LinearVertical);
        sliders[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(sliders[i]);

        labels[i].setJustificationType(juce::Justification::centred);
        labels[i].setColour(juce::Label::textColourId, juce::Colours::goldenrod);
        addAndMakeVisible(labels[i]);
    }

    configureForGroup(currentGroup);
}

void EffectControlPanel::setEffectGroup(EffectGroup group)
{
    if (group == currentGroup)
        return;

    currentGroup = group;
    configureForGroup(group);
    resized();
}

void EffectControlPanel::clearAttachments()
{
    for (auto& a : attachments)
        a.reset();
}

void EffectControlPanel::configureForGroup(EffectGroup group)
{
    clearAttachments();

    struct ParamConfig { juce::String name; juce::String id; };
    std::vector<ParamConfig> config;

    switch (group)
    {
    case EffectGroup::BitcrusherDrive:
        config = { {"Bit Depth", "bitDepth"}, {"Downsample", "downsampleFactor"}, {"Drive", "drive"}, {"Blend", "driveBlend"} };
        break;

    case EffectGroup::GlitchDropout:
        config = { {"Glitch Length", "glitchLength"}, {"Glitch Chance", "glitchChance"}, {"Dropout Rate", "dropoutRate"}, {"Hold Duration", "holdDuration"} };
        break;

    case EffectGroup::Granular:
        config = { {"Grain Size", "grainSize"}, {"Grain Spacing", "grainSpacing"}, {"Playback Rate", "playbackRate"}, {"Jitter", "jitterAmount"} };
        break;

    case EffectGroup::PitchStereo:
        config = { {"Pitch Depth", "pitchDepth"}, {"Pitch Rate", "pitchRate"}, {"Stereo Depth", "stereoDepth"}, {"Stereo Rate", "stereoRate"} };
        break;
    }

    for (int i = 0; i < 4; ++i)
    {
        labels[i].setText(config[i].name, juce::dontSendNotification);
        attachments[i] = std::make_unique<Attachment>(apvts, config[i].id, sliders[i]);
    }
}

void EffectControlPanel::resized()
{
    auto area = getLocalBounds();
    auto h = area.getHeight();

    // Reverted exactly to your original layout logic
    auto padArea = area.removeFromTop(h * .05);
    auto sliderArea = area.removeFromTop(h * .62);
    auto labelArea = area.removeFromBottom(75);

    auto w = sliderArea.getWidth() / 4;

    sliders[0].setBounds(sliderArea.removeFromLeft(w + (w * .04)));
    sliders[1].setBounds(sliderArea.removeFromLeft(w - (w * .02)));
    sliders[2].setBounds(sliderArea.removeFromLeft(w));
    sliders[3].setBounds(sliderArea.removeFromLeft(w - (w * .05)));

    for (int i = 0; i < 4; ++i)
    {
        labels[i].setBounds(labelArea.removeFromLeft(w));
    }
}