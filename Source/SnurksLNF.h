/*
  ==============================================================================
    SnurksLNF.h
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>


namespace juce {


class SnurksLNF : public juce::LookAndFeel_V4
{
public:
    SnurksLNF();

    // === Sliders ===
    void drawLinearSlider(juce::Graphics& g,
        int x, int y, int width, int height,
        float sliderPos,
        float minSliderPos,
        float maxSliderPos,
        const juce::Slider::SliderStyle style,
        juce::Slider& slider) override;

    void drawRotarySlider(juce::Graphics& g,
        int x, int y, int width, int height,
        float sliderPosProportional,
        float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider& slider) override;


	// === ComboBox ===
    void drawComboBox(juce::Graphics& g, int width, int height,
        bool isButtonDown,
        int buttonX, int buttonY,
        int buttonW, int buttonH,
        juce::ComboBox& box) override;

    void drawPopupMenuItem(juce::Graphics& g,
        const juce::Rectangle<int>& area,
        bool isSeparator,
        bool isActive,
        bool isHighlighted,
        bool isTicked,
        bool hasSubMenu,
        const juce::String& text,
        const juce::String& shortcutKeyText,
        const juce::Drawable* icon,
        const juce::Colour* textColour) override;

    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override;
    void getIdealPopupMenuItemSize(const juce::String& text, bool isSeparator,
        int targetHeight, int& idealWidth, int& idealHeight) override;
    

    // === Button ===
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
        const juce::Colour& backgroundColour,
        bool isMouseOverButton, bool isButtonDown) override;
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
        bool isMouseOverButton, bool isButtonDown) override;
    
    
	// === Fonts ===
    juce::Typeface::Ptr getTypefaceForFont(const juce::Font& f) override;
    juce::Font getLabelFont(juce::Label& label) override;




private:
    juce::Image sliderThumb;
	juce::Image rotaryDial;
    juce::Image buttonON;
	juce::Image buttonOFF;
	
    juce::Typeface::Ptr luminariTypeface;

    // ComboBox colors
    juce::Colour comboBoxBackground{ juce::Colours::goldenrod };
    juce::Colour comboBoxOutline{ juce::Colours::darkgoldenrod };
    juce::Colour comboBoxArrow{ juce::Colours::darkolivegreen };
    juce::Colour comboBoxText{ juce::Colours::darkred };
};

}