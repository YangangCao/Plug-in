/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GainAudioProcessorEditor::GainAudioProcessorEditor (AudioProcessor&parent, AudioProcessorValueTreeState& vts)
: AudioProcessorEditor (parent), valueTreeState (vts)
{
    gainLabel.setText ("Gain", dontSendNotification);
    addAndMakeVisible (gainLabel);
    
    addAndMakeVisible (gainSlider);
    gainAttachment.reset (new SliderAttachment (valueTreeState, "gain", gainSlider));
    
    invertButton.setButtonText ("Invert Phase");
    addAndMakeVisible (invertButton);
    invertAttachment.reset (new ButtonAttachment (valueTreeState, "invertPhase", invertButton));
    
    setSize (paramSliderWidth + paramLabelWidth, jmax (100, paramControlHeight * 2));
}

GainAudioProcessorEditor::~GainAudioProcessorEditor()
{
}

//==============================================================================
void GainAudioProcessorEditor::resized()
{
    auto r = getLocalBounds();
    
    auto gainRect = r.removeFromTop (paramControlHeight);
    gainLabel .setBounds (gainRect.removeFromLeft (paramLabelWidth));
    gainSlider.setBounds (gainRect);
    
    invertButton.setBounds (r.removeFromTop (paramControlHeight));
}

void GainAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::black);
}
