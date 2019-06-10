/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"


//==============================================================================
/**
*/
class EqualizerAudioProcessor  : public AudioProcessor,
                                   public AudioProcessorValueTreeState::Listener,
                                   public ChangeBroadcaster
{
public:
    enum FilterType // 滤波器类型，也就是下拉选项
    {
        NoFilter = 0,
        HighPass,
        HighPass1st,
        LowShelf,
        BandPass,
        AllPass,
        AllPass1st,
        Notch,
        Peak,
        HighShelf,
        LowPass1st,
        LowPass,
        LastFilterID
    };

    static String paramOutput;
    static String paramType;
    static String paramFrequency;
    static String paramQuality;
    static String paramGain;
    static String paramActive;

    static String getBandID (size_t index);
    static String getTypeParamName (size_t index);
    static String getFrequencyParamName (size_t index);
    static String getQualityParamName (size_t index);
    static String getGainParamName (size_t index);
    static String getActiveParamName (size_t index);

    //==============================================================================
    EqualizerAudioProcessor();
    ~EqualizerAudioProcessor();

    //==============================================================================
    void prepareToPlay (double newSampleRate, int newSamplesPerBlock) override; // 模版中含有此函数
    void releaseResources() override; // 模版中含有此函数

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override; // 模版中含有此函数
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override; // 模版中含有此函数

    void parameterChanged (const String& parameter, float newValue) override;

    AudioProcessorValueTreeState& getPluginState();

    size_t getNumBands () const;

    String getBandName   (size_t index) const;
    Colour getBandColour (size_t index) const;

    void setBandSolo (int index);
    bool getBandSolo (int index) const;

    static StringArray getFilterTypeNames();

    //==============================================================================
    AudioProcessorEditor* createEditor() override; // 模版中含有此函数
    bool hasEditor() const override; // 模版中含有此函数

    const std::vector<double>& getMagnitudes ();

    void createFrequencyPlot (Path& p, const std::vector<double>& mags, const Rectangle<int> bounds, float pixelsPerDouble); // 绘制滤波器控制曲线

    void createAnalyserPlot (Path& p, const Rectangle<int> bounds, float minFreq, bool input); // 绘制输入输出曲线

    bool checkForNewAnalyserData();

    //==============================================================================
    const String getName() const override; // 模版中含有此函数

    bool acceptsMidi() const override; // 模版中含有此函数
    bool producesMidi() const override; // 模版中含有此函数
    bool isMidiEffect() const override; // 模版中含有此函数
    double getTailLengthSeconds() const override; // 模版中含有此函数

    //==============================================================================
    int getNumPrograms() override; // 模版中含有此函数
    int getCurrentProgram() override; // 模版中含有此函数
    void setCurrentProgram (int index) override; // 模版中含有此函数
    const String getProgramName (int index) override; // 模版中含有此函数
    void changeProgramName (int index, const String& newName) override; // 模版中含有此函数

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override; // 模版中含有此函数
    void setStateInformation (const void* data, int sizeInBytes) override; // 模版中含有此函数
    Point<int> getSavedSize() const;
    void setSavedSize (const Point<int>& size);

    //==============================================================================
    struct Band
    {
        Band (const String& nameToUse, Colour colourToUse, FilterType typeToUse,
            float frequencyToUse, float qualityToUse, float gainToUse=1.0f, bool shouldBeActive=true)
          : name (nameToUse),
            colour (colourToUse),
            type (typeToUse),
            frequency (frequencyToUse),
            quality (qualityToUse),
            gain (gainToUse),
            active (shouldBeActive)
        {}

        String      name;
        Colour      colour;
        FilterType  type      = BandPass;
        float       frequency = 1000.0f;
        float       quality   = 1.0f;
        float       gain      = 1.0f;
        bool        active    = true;
        std::vector<double> magnitudes;
    };

    Band* getBand (size_t index);
    int getBandIndexFromID (String paramID);

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqualizerAudioProcessor)

    void updateBand (const size_t index);

    void updateBypassedStates ();

    void updatePlots ();

    UndoManager                  undo;
    AudioProcessorValueTreeState parameters;

    std::vector<Band>    bands;

    std::vector<double> frequencies;
    std::vector<double> magnitudes;

    bool wasBypassed = true;

    using FilterBand = dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, dsp::IIR::Coefficients<float>>;
    using Gain       = dsp::Gain<float>;
    dsp::ProcessorChain<FilterBand, FilterBand, FilterBand, FilterBand, FilterBand, FilterBand, Gain> filter;

    double sampleRate = 0;

    int soloed = -1;

    Analyser<float> inputAnalyser;
    Analyser<float> outputAnalyser;

    Point<int> editorSize = { 900, 500 };
};
