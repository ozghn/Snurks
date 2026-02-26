///*
//  ==============================================================================
//
//    This file contains the basic framework code for a JUCE plugin editor.
//
//  ==============================================================================
//*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

Snurks_V1AudioProcessorEditor::Snurks_V1AudioProcessorEditor(Snurks_V1AudioProcessor& p)
    : AudioProcessorEditor(&p),
    audioProcessor(p),
    effectPanel(p.apvts)
{
    setLookAndFeel(&lnf);

    // --- 1. Utility for repeated UI setup ---
    auto setupRotary = [this](juce::Slider& s, juce::Label& l, const juce::String& name) {
        s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(s);

        l.setText(name, juce::dontSendNotification);
        l.setFont(juce::Font(22.0f));
        l.setColour(juce::Label::textColourId, juce::Colours::goldenrod);
        l.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(l);
        };

    setupRotary(fadeInSlider, fadeLabel, "Fade In");
    setupRotary(rampSpeedSlider, rampLabel, "Bit Ramp");

    // --- 2. Effect Selector ---
    effectSelector.addItemList({ "Bitcrusher + Drive", "Glitch + Dropout", "Granular", "Pitch + Stereo" }, 1);

    effectSelector.onChange = [this] {
        // Casts ID directly to the enum
        auto group = static_cast<EffectGroup>(effectSelector.getSelectedId() - 1);
        effectPanel.setEffectGroup(group);
        };

    effectSelector.setSelectedId(1);
    addAndMakeVisible(effectSelector);

    // --- 3. Main Toggle (SNURK) ---
    enableButton.setButtonText("");
    enableButton.setClickingTogglesState(true);
    addAndMakeVisible(enableButton);

    enableLabel.setText("SNURK", juce::dontSendNotification);
    enableLabel.setFont(juce::Font(30.0f));
    enableLabel.setColour(juce::Label::textColourId, juce::Colours::goldenrod);
    enableLabel.setJustificationType(juce::Justification::centred);
    enableLabel.addMouseListener(&enableButton, false);
    addAndMakeVisible(enableLabel);

    // --- 4. Parameter Attachments ---
    fadeInAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "fadeInDuration", fadeInSlider);
    rampSpeedAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "bitDepthRampSpeed", rampSpeedSlider);
    enableAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "enableEffect", enableButton);

    // --- 5. Effect Panel & Background ---
    addAndMakeVisible(effectPanel);
    snurkBackground = juce::ImageCache::getFromMemory(BinaryData::snurk_png, BinaryData::snurk_pngSize);

    setSize(1227, 704);
    
}

Snurks_V1AudioProcessorEditor::~Snurks_V1AudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void Snurks_V1AudioProcessorEditor::paint(juce::Graphics& g)
{
    g.drawImage(snurkBackground, getLocalBounds().toFloat(), juce::RectanglePlacement::stretchToFit);

    // Debug helper for panel alignment
    // g.setColour(juce::Colours::magenta.withAlpha(0.3f));
    // g.drawRect(effectPanel.getBounds(), 1.0f);
}

void Snurks_V1AudioProcessorEditor::resized()
{
    const auto w = getWidth();
    const auto h = getHeight();

    // Macro Controls
    fadeInSlider.setBounds(w * 0.15f, h * 0.60f, w * 0.09f, w * 0.09f);
    fadeLabel.setBounds(fadeInSlider.getX(), h * 0.775f, fadeInSlider.getWidth(), 35);

    rampSpeedSlider.setBounds(w * 0.30f, h * 0.60f, w * 0.09f, w * 0.09f);
    rampLabel.setBounds(rampSpeedSlider.getX(), h * 0.775f, rampSpeedSlider.getWidth(), 35);

    // Snurk Toggle
    enableButton.setBounds(w * 0.14f, h * 0.22f, w * 0.10f, h * 0.10f);
    enableLabel.setBounds(w * 0.21f, h * 0.24f, enableButton.getWidth(), 35);

    // Selector & Panel
    effectSelector.setBounds(w * 0.171f, h * 0.368f, w * 0.165f, h * 0.089f);
    effectPanel.setBounds(w * 0.486f, h * 0.35f, w * 0.39f, h * 0.48f);
}
