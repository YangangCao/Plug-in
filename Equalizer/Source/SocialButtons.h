#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class SocialButtons    : public Component, // The base class for all JUCE user-interface objects.
                         public Button::Listener // Used to receive callbacks when a button is clicked.
{
public:
    SocialButtons()
    {
        setOpaque (false); // Indicates whether any parts of the component might be transparent.

        auto* b = buttons.add (new ImageButton()); // Creates an ImageButton.
        b->addListener (this); // Registers a listener to receive events when this button's state changes.
        auto ffLogo = ImageCache::getFromMemory (DNVAudioData::LogoDNV_png, DNVAudioData::LogoDNV_pngSize); // Loads an image from an in-memory image file, (or just returns the image if it's already cached).
        b->setImages (false, true, true, ffLogo, 1.0f, Colours::transparentWhite, ffLogo, 0.7f, Colours::transparentWhite, ffLogo, 0.7f, Colours::transparentWhite); // Sets up the images to draw in various states.
        b->setComponentID ("https://vfinemusic.com/"); // Sets the component's ID string.
        b->setTooltip (TRANS ("Go to the DNV Music Group Website \"vfinemusic.com\"")); // Sets the tooltip for this button.
        addAndMakeVisible (b); // Adds a child component to this one, and also makes the child visible if it isn't already.

        b = buttons.add (new ImageButton());
        b->addListener (this);
        auto fbLogo = ImageCache::getFromMemory (DNVAudioData::FBlogo_png, DNVAudioData::FBlogo_pngSize);
        b->setImages (false, true, true, fbLogo, 1.0f, Colours::transparentWhite, fbLogo, 0.7f, Colours::transparentWhite, fbLogo, 0.7f, Colours::transparentWhite);
        b->setComponentID ("https://www.fb.com/FoleysFinest/");
        b->setTooltip (TRANS ("Like or connect with us on Facebook"));
        addAndMakeVisible (b);

        b = buttons.add (new ImageButton());
        b->addListener (this);
        auto inLogo = ImageCache::getFromMemory (DNVAudioData::Inlogo_png, DNVAudioData::Inlogo_pngSize);
        b->setImages (false, true, true, inLogo, 1.0f, Colours::transparentWhite, inLogo, 0.7f, Colours::transparentWhite, inLogo, 0.7f, Colours::transparentWhite);
        b->setComponentID ("https://www.linkedin.com/company/dnv-music-group");
        b->setTooltip (TRANS ("See our profile on Linked.In (TM)"));
        addAndMakeVisible (b);

        b = buttons.add (new ImageButton());
        b->addListener (this);
        auto gitlabLogo = ImageCache::getFromMemory (DNVAudioData::GitLablogo_png, DNVAudioData::GitLablogo_pngSize);
        b->setImages (false, true, true, gitlabLogo, 1.0f, Colours::transparentWhite, gitlabLogo, 0.7f, Colours::transparentWhite, gitlabLogo, 0.7f, Colours::transparentWhite);
        b->setComponentID ("https://gitlab.com/YangangCao");
        b->setTooltip (TRANS ("Find resources on GitLab"));
        addAndMakeVisible (b);
    }

    ~SocialButtons()
    {
    }

    void paint (Graphics& g) override
    {
        auto renderedText = ImageCache::getFromMemory (DNVAudioData::DNVtext_png, DNVAudioData::DNVtext_pngSize);
        g.drawImageWithin (renderedText, 0, 0, getWidth(), getHeight(), RectanglePlacement (RectanglePlacement::xRight)); // Draws an image to fit within a designated rectangle.
    }

    void resized() override
    {
        auto bounds = getLocalBounds(); // Returns the component's bounds, relative to its own origin.
        for (auto* b : buttons)
            b->setBounds (bounds.removeFromLeft (bounds.getHeight()).reduced (3)); // Removes a strip from the left-hand edge of this rectangle, reducing this rectangle by the specified amount and returning the section that was removed. E.g. if this rectangle is (100, 100, 300, 300) and amountToRemove is 50, this will return (100, 100, 50, 300) and leave this rectangle as (150, 100, 250, 300).
        // Returns a rectangle that is smaller than this one by a given amount. reduced 的作用是让左下角的图片变小并且之间有点间隔
    }

    void buttonClicked (Button* b) override
    {
        URL url (b->getComponentID());
        if (url.isWellFormed()) // True if it seems to be valid.
        {
            url.launchInDefaultBrowser(); // Tries to launch the system's default browser to open the URL. Returns true if this seems to have worked.
        }
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SocialButtons)
    OwnedArray<ImageButton> buttons; // An array designed for holding objects.

};
