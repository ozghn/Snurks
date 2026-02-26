/*
  ==============================================================================
    SnurksLNF.cpp
*/

#include "SnurksLNF.h"

namespace juce {

    SnurksLNF::SnurksLNF()
    {
        sliderThumb = juce::ImageCache::getFromMemory(BinaryData::st_png, BinaryData::st_pngSize);
		rotaryDial = juce::ImageCache::getFromMemory(BinaryData::rd_png, BinaryData::rd_pngSize);
		buttonON = juce::ImageCache::getFromMemory(BinaryData::btON_png, BinaryData::btON_pngSize);
		buttonOFF = juce::ImageCache::getFromMemory(BinaryData::btOFF_png, BinaryData::btOFF_pngSize);
		
		luminariTypeface = juce::Typeface::createSystemTypefaceFor(BinaryData::LuminariRegular_ttf, BinaryData::LuminariRegular_ttfSize);
    }

    // === Sliders ===

    void SnurksLNF::drawLinearSlider(juce::Graphics& g,
        int x, int y, int width, int height,
        float sliderPos,
        float /*minSliderPos*/,
        float /*maxSliderPos*/,
        const juce::Slider::SliderStyle style,
        juce::Slider& /*slider*/)
    {
        if (!sliderThumb.isValid())
            return;

        Rectangle<float> thumbBounds;

        // --- Determine thumb size ---
        float maxThumbW = jmin(30.0f, (float)width * 0.8f);  // max 30px or 80% of slider width
        float maxThumbH = jmin(30.0f, (float)height * 0.8f); // max 30px or 80% of slider height

        // Preserve image aspect ratio
        float imgW = (float)sliderThumb.getWidth();
        float imgH = (float)sliderThumb.getHeight();
        float aspect = imgW / imgH;

        float thumbW = maxThumbW;
        float thumbH = maxThumbH;

        if (aspect > 1.0f)
        {
            // Image is wider than tall
            thumbH = thumbW / aspect;
        }
        else
        {
            // Image is taller than wide
            thumbW = thumbH * aspect;
        }

        float halfThumbW = thumbW * 0.5f;
        float halfThumbH = thumbH * 0.5f;

        // --- Calculate thumb bounds and clamp to slider ---
        if (style == Slider::LinearVertical)
        {
            float top = y + halfThumbH;
            float bottom = y + height - halfThumbH;
            float clampedPos = juce::jlimit(top, bottom, sliderPos);

            thumbBounds = Rectangle<float>(
                x + (width - thumbW) * 0.5f,
                clampedPos - halfThumbH,
                thumbW,
                thumbH
            );
        }
        else if (style == Slider::LinearHorizontal)
        {
            float left = x + halfThumbW;
            float right = x + width - halfThumbW;
            float clampedPos = juce::jlimit(left, right, sliderPos);

            thumbBounds = Rectangle<float>(
                clampedPos - halfThumbW,
                y + (height - thumbH) * 0.5f,
                thumbW,
                thumbH
            );
        }
        else
        {
            // Other slider styles: do nothing
            return;
        }

        // --- Draw the thumb ---
        g.drawImage(sliderThumb, thumbBounds);
    }

    void SnurksLNF::drawRotarySlider(juce::Graphics& g,
        int x, int y, int width, int height,
        float sliderPosProportional,
        float /*rotaryStartAngle*/,
        float /*rotaryEndAngle*/,
        juce::Slider& /*slider*/)
    {
        if (!rotaryDial.isValid())
            return;

        // Determine maximum knob size (50-60% of smaller dimension)
        float knobSize = jmin((float)width * 0.95f, (float)height) * 0.4f;

        // Preserve aspect ratio
        float imgW = (float)rotaryDial.getWidth()/2;
        float imgH = (float)rotaryDial.getHeight();
        float aspect = imgW / imgH;

        float drawW = knobSize;
        float drawH = knobSize;

        if (aspect > 1.0f)
            drawH = drawW / aspect;
        else
            drawW = drawH * aspect;

        // Slider center
        float centerX = x + width * 0.5f;
        float centerY = y + height * 0.5f;

        // Top-left position to draw image so top aligns with center
        float drawX = centerX - drawW * 0.5f;
        float drawY = centerY;  // top of image is at slider center

        // Pivot at top-center
        float pivotX = drawX + drawW * 0.5f;
        float pivotY = drawY; // top of image

        // Compute rotation angle
        const float startAngle = juce::MathConstants<float>::pi / 4.0f;  // 225°
        const float endAngle = 7.0f * juce::MathConstants<float>::pi / 4.0f;  // 315°

        float angle = startAngle + sliderPosProportional * (endAngle - startAngle);

        g.addTransform(juce::AffineTransform::rotation(angle, pivotX, pivotY));
        g.drawImage(rotaryDial, drawX, drawY, drawW, drawH,
            0, 0, rotaryDial.getWidth(), rotaryDial.getHeight());
    }

    // === ComboBox ===
    void SnurksLNF::drawComboBox(juce::Graphics& g, int width, int height,
        bool /*isButtonDown*/,
        int buttonX, int buttonY,
        int buttonW, int buttonH,
        juce::ComboBox& box)
    {
        auto bounds = juce::Rectangle<int>(0, 0, width, height);

        // Background
        g.setColour(comboBoxBackground.withAlpha(0.1f));
        g.fillRoundedRectangle(bounds.toFloat(), 6.0f);

        // Outline
        g.setColour(comboBoxOutline.withAlpha(0.1f));
        g.drawRoundedRectangle(bounds.toFloat(), 6.0f, 1.5f);

        // Draw the ComboBox text — prevent double-draw by clipping
        g.setColour(comboBoxText);
        g.setFont(14.0f);

        // Clip the drawing area so JUCE doesn’t draw text over it internally
        g.reduceClipRegion(bounds);


        // Draw the arrow
        float arrowSize = 10.0f;
        juce::Path arrow;
        arrow.addTriangle(
            buttonX + buttonW / 2 - arrowSize / 2.0f,
            buttonY + buttonH * 0.4f,
            buttonX + buttonW / 2 + arrowSize / 2.0f,
            buttonY + buttonH * 0.4f,
            buttonX + buttonW / 2,
            buttonY + buttonH * 0.6f
        );

        g.setColour(comboBoxArrow);
        g.fillPath(arrow);
    }

    // === Draw popup menu items ===
    void SnurksLNF::drawPopupMenuItem(juce::Graphics& g,
        const juce::Rectangle<int>& area,
        bool isSeparator,
        bool /*isActive*/,
        bool isHighlighted,
        bool isTicked,
        bool hasSubMenu,
        const juce::String& text,
        const juce::String& shortcutKeyText,
        const juce::Drawable* icon,
        const juce::Colour* textColour)
    {
        
        auto r = area.toFloat();

        if (isSeparator)
        {
            g.setColour(juce::Colours::darkgoldenrod);
            g.drawLine(r.getX(), r.getCentreY(), r.getRight(), r.getCentreY(), 1.0f);
            return;
        }

        // Background
        g.setColour(isHighlighted ? juce::Colours::goldenrod : comboBoxBackground);
        g.fillRect(r);

        // Text
        g.setColour(isHighlighted ? juce::Colours::black : comboBoxText);
        g.setFont(14.0f);
        g.drawFittedText(text,
            area.reduced(4, 0).toNearestInt(),
            juce::Justification::centredLeft,
            1);

        // Tick mark
        if (isTicked)
        {
            g.setColour(comboBoxArrow); // Use arrow color for tick
            g.drawLine(r.getRight() - 10.0f, r.getCentreY() - 3.0f,
                r.getRight() - 5.0f, r.getCentreY() + 3.0f, 2.0f);
            g.drawLine(r.getRight() - 10.0f, r.getCentreY() + 3.0f,
                r.getRight() - 5.0f, r.getCentreY() - 3.0f, 2.0f);
        }
    }

    void SnurksLNF::positionComboBoxText(juce::ComboBox& box, juce::Label& label)
    {
        // 1. Position it inside the box (adjust the '5' to pad the text from the left)
        label.setBounds(5, 0, box.getWidth() - 30, box.getHeight());

        // 2. FORCE the color directly on the label instance
        label.setColour(juce::Label::textColourId, juce::Colours::goldenrod);

        // 3. Optional: match the font from your drawComboBox
        label.setFont(juce::Font(22.0f));
    }

    void SnurksLNF::getIdealPopupMenuItemSize(const juce::String& text, bool isSeparator,
        int targetHeight, int& idealWidth, int& idealHeight)
    {
        // 1. Force the height of the row (this makes the menu shorter/taller)
        idealHeight = 24;

        // 2. Force the width (this makes the menu narrower/wider)
        auto font = getPopupMenuFont();
        idealWidth = font.getStringWidth(text) + 30; // 30px padding for the tick/arrow
    }

    
    // === Button ===
    void SnurksLNF::drawButtonBackground(juce::Graphics& g, juce::Button& button,
        const juce::Colour& backgroundColour,
        bool isMouseOverButton, bool isButtonDown)
    {
        // 1. Determine which image to use based on the toggle state
        // We use getToggleState() for "On/Off" behavior
        juce::Image& imageToDraw = button.getToggleState() ? buttonON : buttonOFF;

        if (!imageToDraw.isValid())
            return;

        auto bounds = button.getLocalBounds().toFloat();

        // 2. Add visual feedback for mouse interaction
        if (isButtonDown)
        {
            g.setOpacity(0.8f);
        }
        else if (isMouseOverButton)
        {
            g.setOpacity(1.0f); // Fully bright on hover
        }
        else
        {
            g.setOpacity(0.9f); // Slightly dimmed when idle
        }

        // 3. Draw the chosen image
        g.drawImageWithin(imageToDraw,
            0, 0, (int)bounds.getWidth(), (int)bounds.getHeight(),
            juce::RectanglePlacement::centred);
    }
    
    void SnurksLNF::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
        bool isMouseOverButton, bool isButtonDown)
    {
        // 1. Pick the image based on state
        auto& img = button.getToggleState() ? buttonON : buttonOFF;

        if (img.isValid())
        {
            auto bounds = button.getLocalBounds().toFloat();

            // Visual feedback
            g.setOpacity(isMouseOverButton ? 1.0f : 0.9f);

            // 2. Draw the image to fill the whole button area
            g.drawImageWithin(img,
                0, 0, (int)bounds.getWidth(), (int)bounds.getHeight(),
                juce::RectanglePlacement::centred);
        }
        else
        {
            // Fallback: If images didn't load, draw a colored box so you know it's working
            g.setColour(juce::Colours::red);
            g.fillRect(button.getLocalBounds());
        }
    }


	// === Fonts ===

    juce::Typeface::Ptr SnurksLNF::getTypefaceForFont(const juce::Font& f)
    {
        //juce::Font customFont("Bauhaus 93", f.getHeight(), f.getStyleFlags());
        //return customFont.getTypefacePtr();
        return luminariTypeface;

    }

    juce::Font SnurksLNF::getLabelFont(juce::Label& label)
    {
        // This forces EVERY label (including the one inside the ComboBox) 
        // to use this specific Font object.
        //float currentHeight = label.getFont().getHeight();
        //return juce::Font("Bauhaus 93", currentHeight, juce::Font::plain);
        return juce::Font(luminariTypeface).withHeight(label.getFont().getHeight());
    }


}
