/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

class GainAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    enum
    {
        paramControlHeight = 40,
        paramLabelWidth    = 80,
        paramSliderWidth   = 300
    };
    typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
    
    GainAudioProcessorEditor (AudioProcessor& parent, AudioProcessorValueTreeState& vts);
    ~GainAudioProcessorEditor();
    
    void paint (Graphics&) override;
    void resized() override;
    
private:
    AudioProcessorValueTreeState& valueTreeState;
    
    Label gainLabel;
    Slider gainSlider;
    std::unique_ptr<SliderAttachment> gainAttachment;
    
    ToggleButton invertButton;
    std::unique_ptr<ButtonAttachment> invertAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GainAudioProcessorEditor)
};
