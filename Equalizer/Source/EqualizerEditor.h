/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/


#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "EqualizerProcessor.h"


//==============================================================================
/**
*/
class EqualizerAudioProcessorEditor  : public AudioProcessorEditor,
                                         public ChangeListener,
                                         public Timer
{
public:
    EqualizerAudioProcessorEditor (EqualizerAudioProcessor&);
    ~EqualizerAudioProcessorEditor();

    //==============================================================================

    void paint (Graphics&) override; // 模版有此函数
    void resized() override; // 模版有此函数
    void changeListenerCallback (ChangeBroadcaster* sender) override;
    void timerCallback() override;

    void mouseDown (const MouseEvent& e) override;

    void mouseMove (const MouseEvent& e) override;
    void mouseDrag (const MouseEvent& e) override;

    void mouseDoubleClick (const MouseEvent& e) override;

    //==============================================================================

    class BandEditor : public Component,
                       public Button::Listener
    {
    public:
        BandEditor (size_t i, EqualizerAudioProcessor& processor);

        void resized () override;

        void updateControls (EqualizerAudioProcessor::FilterType type);

        void updateSoloState (bool isSolo);

        void setFrequency (float frequency);

        void setGain (float gain);

        void setType (int type);

        void buttonClicked (Button* b) override;

        Path frequencyResponse;
    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BandEditor)

        size_t index;
        EqualizerAudioProcessor& processor;

        GroupComponent      frame;
        ComboBox            filterType;
        Slider              frequency;
        Slider              quality;
        Slider              gain;
        TextButton          solo;
        TextButton          activate;
        OwnedArray<AudioProcessorValueTreeState::ComboBoxAttachment> boxAttachments;
        OwnedArray<AudioProcessorValueTreeState::SliderAttachment> attachments;
        OwnedArray<AudioProcessorValueTreeState::ButtonAttachment> buttonAttachments;
    };

private:

    void updateFrequencyResponses ();

    static float getPositionForFrequency (float freq);

    static float getFrequencyForPosition (float pos);

    static float getPositionForGain (float gain, float top, float bottom);

    static float getGainForPosition (float pos, float top, float bottom);

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    EqualizerAudioProcessor& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqualizerAudioProcessorEditor)

#ifdef JUCE_OPENGL
    OpenGLContext           openGLContext; // Creates an OpenGL context, which can be attached to a component.
#endif

    OwnedArray<BandEditor>  bandEditors;

    Rectangle<int>          plotFrame; // 绘图区域
    Rectangle<int>          brandingFrame; // 滤波器组区域

    Path                    frequencyResponse;
    Path                    analyserPath;

    GroupComponent          frame;
    Slider                  output;

    SocialButtons           socialButtons;

    int                     draggingBand = -1;
    bool                    draggingGain = false;

    OwnedArray<AudioProcessorValueTreeState::SliderAttachment> attachments;
    SharedResourcePointer<TooltipWindow> tooltipWindow;

    PopupMenu               contextMenu;
};
