/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/
#include "Analyser.h"
#include "EqualizerProcessor.h"
#include "SocialButtons.h"
#include "EqualizerEditor.h"


String EqualizerAudioProcessor::paramOutput   ("output");
String EqualizerAudioProcessor::paramType     ("type");
String EqualizerAudioProcessor::paramFrequency("frequency");
String EqualizerAudioProcessor::paramQuality  ("quality");
String EqualizerAudioProcessor::paramGain     ("gain");
String EqualizerAudioProcessor::paramActive   ("active");

namespace IDs
{
    String editor {"editor"};
    String sizeX  {"size-x"};
    String sizeY  {"size-y"};
}

String EqualizerAudioProcessor::getBandID (size_t index)
{
    switch (index)
    {
        case 0: return "Lowest";
        case 1: return "Low";
        case 2: return "Low Mids";
        case 3: return "High Mids";
        case 4: return "High";
        case 5: return "Highest";
        default: break;
    }
    return "unknown";
}

int EqualizerAudioProcessor::getBandIndexFromID (String paramID)
{
    for (size_t i=0; i < 6; ++i)
        if (paramID.startsWith (getBandID (i) + "-"))
            return int (i);

    return -1;
}

std::vector<EqualizerAudioProcessor::Band> createDefaultBands() // 设置默filters的默认值,和后面函数无过多耦合，只会调用一次创建默认band
{
    std::vector<EqualizerAudioProcessor::Band> defaults;
    defaults.push_back (EqualizerAudioProcessor::Band (TRANS ("Lowest"),    Colours::blue,   EqualizerAudioProcessor::HighPass,    20.0f, 0.707f));
    defaults.push_back (EqualizerAudioProcessor::Band (TRANS ("Low"),       Colours::brown,  EqualizerAudioProcessor::LowShelf,   250.0f, 0.707f));
    defaults.push_back (EqualizerAudioProcessor::Band (TRANS ("Low Mids"),  Colours::green,  EqualizerAudioProcessor::Peak,       500.0f, 0.707f));
    defaults.push_back (EqualizerAudioProcessor::Band (TRANS ("High Mids"), Colours::coral,  EqualizerAudioProcessor::Peak,      1000.0f, 0.707f));
    defaults.push_back (EqualizerAudioProcessor::Band (TRANS ("High"),      Colours::orange, EqualizerAudioProcessor::HighShelf, 5000.0f, 0.707f));
    defaults.push_back (EqualizerAudioProcessor::Band (TRANS ("Highest"),   Colours::red,    EqualizerAudioProcessor::LowPass,  12000.0f, 0.707f));
    return defaults;
}

AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<AudioProcessorParameterGroup>> params;

    // setting defaults
    const float maxGain = Decibels::decibelsToGain (24.0f);
    auto defaults = createDefaultBands();

    {   // Output
        auto param = std::make_unique<AudioParameterFloat> (EqualizerAudioProcessor::paramOutput, TRANS ("Output"),
                                                            NormalisableRange<float> (0.0f, 2.0f, 0.01f), 1.0f,
                                                            TRANS ("Output level"),
                                                            AudioProcessorParameter::genericParameter,
                                                            [](float value, int) {return String (Decibels::gainToDecibels(value), 1) + " dB";},
                                                            [](String text) {return Decibels::decibelsToGain (text.dropLastCharacters (3).getFloatValue());});

        auto group = std::make_unique<AudioProcessorParameterGroup> ("global", TRANS ("Globals"), "|", std::move (param));
        params.push_back (std::move (group));
    }

    for (size_t i = 0; i < defaults.size(); ++i)
    {
        auto typeParameter = std::make_unique<AudioParameterChoice> (EqualizerAudioProcessor::getTypeParamName (i),
                                                                     TRANS ("Filter Type"),
                                                                     EqualizerAudioProcessor::getFilterTypeNames(),
                                                                     defaults [i].type);

        auto freqParameter = std::make_unique<AudioParameterFloat> (EqualizerAudioProcessor::getFrequencyParamName (i), TRANS ("Frequency"),
                                                                    NormalisableRange<float> {20.0f, 20000.0f, 1.0f},
                                                                    defaults [i].frequency,
                                                                    TRANS ("Frequency"),
                                                                    AudioProcessorParameter::genericParameter,
                                                                    [](float value, int) { return (value < 1000) ?
                                                                        String (value, 0) + " Hz" :
                                                                        String (value / 1000.0, 2) + " kHz"; },
                                                                    [](String text) { return text.endsWith(" kHz") ?
                                                                        text.dropLastCharacters (4).getFloatValue() * 1000.0 :
                                                                        text.dropLastCharacters (3).getFloatValue(); });

        auto qltyParameter = std::make_unique<AudioParameterFloat> (EqualizerAudioProcessor::getQualityParamName (i), TRANS ("Quality"),
                                                                    NormalisableRange<float> {0.1f, 10.0f, 1.0f},
                                                                    defaults [i].quality,
                                                                    TRANS ("Quality"),
                                                                    AudioProcessorParameter::genericParameter,
                                                                    [](float value, int) { return String (value, 1); },
                                                                    [](const String& text) { return text.getFloatValue(); });

        auto gainParameter = std::make_unique<AudioParameterFloat> (EqualizerAudioProcessor::getGainParamName (i), TRANS ("Gain"),
                                                                    NormalisableRange<float> {1.0f / maxGain, maxGain, 0.001f},
                                                                    defaults [i].gain,
                                                                    TRANS ("Band Gain"),
                                                                    AudioProcessorParameter::genericParameter,
                                                                    [](float value, int) {return String (Decibels::gainToDecibels(value), 1) + " dB";},
                                                                    [](String text) {return Decibels::decibelsToGain (text.dropLastCharacters (3).getFloatValue());});

        auto actvParameter = std::make_unique<AudioParameterBool> (EqualizerAudioProcessor::getActiveParamName (i), TRANS ("Active"),
                                                                   defaults [i].active,
                                                                   TRANS ("Band Active"),
                                                                   [](float value, int) {return value > 0.5f ? TRANS ("active") : TRANS ("bypassed");},
                                                                   [](String text) {return text == TRANS ("active");});

        auto group = std::make_unique<AudioProcessorParameterGroup> ("band" + String (i), defaults [i].name, "|",
                                                                     std::move (typeParameter),
                                                                     std::move (freqParameter),
                                                                     std::move (qltyParameter),
                                                                     std::move (gainParameter),
                                                                     std::move (actvParameter));

        params.push_back (std::move (group));
    }

    return { params.begin(), params.end() };
}

//==============================================================================
EqualizerAudioProcessor::EqualizerAudioProcessor() :
#ifndef JucePlugin_PreferredChannelConfigurations
    AudioProcessor (BusesProperties()
                    .withInput  ("Input",  AudioChannelSet::stereo(), true)
                    .withOutput ("Output", AudioChannelSet::stereo(), true)
                    ),
#endif
parameters (*this, &undo, "PARAMS", createParameterLayout()) // AudioProcessorValueTreeState parameters;
{
    frequencies.resize (300); // 分配vector大小
    for (size_t i=0; i < frequencies.size(); ++i)
    {
        frequencies [i] = 20.0 * std::pow (2.0, i / 30.0);
    }
    magnitudes.resize (frequencies.size()); // 设为和frequencies相同的值

    // 需要与ProcessorChain filter同步
    bands = createDefaultBands(); // bands是一个装着所有band的vector

    for (size_t i = 0; i < bands.size(); ++i)
    {
        bands [i].magnitudes.resize (frequencies.size(), 1.0); // 1.0用来初始化

        parameters.addParameterListener (getTypeParamName (i), this); // (Parameter ID, listener)
        parameters.addParameterListener (getFrequencyParamName (i), this);
        parameters.addParameterListener (getQualityParamName (i), this);
        parameters.addParameterListener (getGainParamName (i), this);
        parameters.addParameterListener (getActiveParamName (i), this);
    }

    parameters.addParameterListener (paramOutput, this);

    parameters.state = ValueTree (JucePlugin_Name);
}

EqualizerAudioProcessor::~EqualizerAudioProcessor()
{
    inputAnalyser.stopThread (1000); // 尝试停止线程，这个函数会令threadShouldExit()返回True，假如线程正在等待就调用notify()
    outputAnalyser.stopThread (1000);
}

//==============================================================================
const String EqualizerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EqualizerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EqualizerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EqualizerAudioProcessor::isMidiEffect() const
{
    return false;
}

double EqualizerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EqualizerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int EqualizerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EqualizerAudioProcessor::setCurrentProgram (int)
{
}

const String EqualizerAudioProcessor::getProgramName (int)
{
    return {};
}

void EqualizerAudioProcessor::changeProgramName (int, const String&)
{
}

//==============================================================================
void EqualizerAudioProcessor::prepareToPlay (double newSampleRate, int newSamplesPerBlock)
{
    sampleRate = newSampleRate;

    dsp::ProcessSpec spec;
    spec.sampleRate = newSampleRate;
    spec.maximumBlockSize = uint32 (newSamplesPerBlock);
    spec.numChannels = uint32 (getTotalNumOutputChannels ());

    for (size_t i=0; i < bands.size(); ++i)
    {
        updateBand (i);
    }
    
    filter.get<6>().setGainLinear (*parameters.getRawParameterValue (paramOutput));

    updatePlots();

    filter.prepare (spec);

    inputAnalyser.setupAnalyser  (int (sampleRate), float (sampleRate));
    outputAnalyser.setupAnalyser (int (sampleRate), float (sampleRate));
}

void EqualizerAudioProcessor::releaseResources()
{
    inputAnalyser.stopThread (1000);
    outputAnalyser.stopThread (1000);
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EqualizerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // This checks if the input layout matches the output layout
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}
#endif

void EqualizerAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals; // Helper class providing an RAII-based mechanism for temporarily disabling denormals on your CPU
    ignoreUnused (midiMessages);

    if (getActiveEditor() != nullptr)
        inputAnalyser.addAudioData (buffer, 0, getTotalNumInputChannels()); // 输入数据

    if (wasBypassed)
    {
        filter.reset();
        wasBypassed = false;
    }
    
    dsp::AudioBlock<float>              ioBuffer (buffer);
    dsp::ProcessContextReplacing<float> context  (ioBuffer); // Contains context information that is passed into an algorithm's process method.
    filter.process (context); // 允许将任意数量的processor连接到一个procrssor中，该process将按顺序调用所有处理器上的process()。

    if (getActiveEditor() != nullptr)
        outputAnalyser.addAudioData (buffer, 0, getTotalNumOutputChannels()); // 输出数据
}

AudioProcessorValueTreeState& EqualizerAudioProcessor::getPluginState()
{
    return parameters;
}

String EqualizerAudioProcessor::getTypeParamName (size_t index)
{
    return getBandID (index) + "-" + paramType;
}

String EqualizerAudioProcessor::getFrequencyParamName (size_t index)
{
    return getBandID (index) + "-" + paramFrequency;
}

String EqualizerAudioProcessor::getQualityParamName (size_t index)
{
    return getBandID (index) + "-" + paramQuality;
}

String EqualizerAudioProcessor::getGainParamName (size_t index)
{
    return getBandID (index) + "-" + paramGain;
}

String EqualizerAudioProcessor::getActiveParamName (size_t index)
{
    return getBandID (index) + "-" + paramActive;
}

void EqualizerAudioProcessor::parameterChanged (const String& parameter, float newValue) // String表示需要改变的参数的名字，float表示改变后的值
{
    if (parameter == paramOutput)
    {
        filter.get<6>().setGainLinear (newValue);
        updatePlots();
        return;
    }

    int index = getBandIndexFromID (parameter);
    if (isPositiveAndBelow (index, bands.size()))
    {
        auto* band = getBand (size_t (index));
        if (parameter.endsWith (paramType))
        {
            band->type = static_cast<FilterType> (static_cast<int> (newValue));
        }
        else if (parameter.endsWith (paramFrequency))
        {
            band->frequency = newValue;
        }
        else if (parameter.endsWith (paramQuality))
        {
            band->quality = newValue;
        }
        else if (parameter.endsWith (paramGain))
        {
            band->gain = newValue;
        }
        else if (parameter.endsWith (paramActive))
        {
            band->active = newValue >= 0.5f;
        }

        updateBand (size_t (index));
    }
}

size_t EqualizerAudioProcessor::getNumBands () const
{
    return bands.size();
}

String EqualizerAudioProcessor::getBandName   (size_t index) const
{
    if (isPositiveAndBelow (index, bands.size()))
        return bands [size_t (index)].name;
    return TRANS ("unknown");
}
Colour EqualizerAudioProcessor::getBandColour (size_t index) const
{
    if (isPositiveAndBelow (index, bands.size()))
        return bands [size_t (index)].colour;
    return Colours::silver;
}

bool EqualizerAudioProcessor::getBandSolo (int index) const
{
    return index == soloed;
}

void EqualizerAudioProcessor::setBandSolo (int index)
{
    soloed = index;
    updateBypassedStates();
}

void EqualizerAudioProcessor::updateBypassedStates ()
{
    if (isPositiveAndBelow (soloed, bands.size())) // if 0 =< soloed < bands.size()
    {
        filter.setBypassed<0>(soloed != 0); // soloed等于几就不跳过几，跳过其他的
        filter.setBypassed<1>(soloed != 1);
        filter.setBypassed<2>(soloed != 2);
        filter.setBypassed<3>(soloed != 3);
        filter.setBypassed<4>(soloed != 4);
        filter.setBypassed<5>(soloed != 5);
    }
    
    else
    {
        filter.setBypassed<0>(!bands[0].active); // 如果soloed == -1，就不跳过任何filter
        filter.setBypassed<1>(!bands[1].active);
        filter.setBypassed<2>(!bands[2].active);
        filter.setBypassed<3>(!bands[3].active);
        filter.setBypassed<4>(!bands[4].active);
        filter.setBypassed<5>(!bands[5].active);
    }
    updatePlots();
}

EqualizerAudioProcessor::Band* EqualizerAudioProcessor::getBand (size_t index)
{
    if (isPositiveAndBelow (index, bands.size()))
        return &bands [index];
    return nullptr;
}

StringArray EqualizerAudioProcessor::getFilterTypeNames() // A special array for holding a list of strings.
{
    return {
        TRANS ("No Filter"),
        TRANS ("High Pass"),
        TRANS ("1st High Pass"),
        TRANS ("Low Shelf"),
        TRANS ("Band Pass"),
        TRANS ("All Pass"),
        TRANS ("1st All Pass"),
        TRANS ("Notch"),
        TRANS ("Peak"),
        TRANS ("High Shelf"),
        TRANS ("1st Low Pass"),
        TRANS ("Low Pass")
    };
}

void EqualizerAudioProcessor::updateBand (const size_t index) // 更新不同滤波器
{
    if (sampleRate > 0)
    {
        dsp::IIR::Coefficients<float>::Ptr newCoefficients;
        switch (bands [index].type)
        {
            case NoFilter:
                newCoefficients = new dsp::IIR::Coefficients<float> (1, 0, 1, 0);
                break;
            case LowPass:
                newCoefficients = dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, bands [index].frequency, bands [index].quality);
                break;
            case LowPass1st:
                newCoefficients = dsp::IIR::Coefficients<float>::makeFirstOrderLowPass (sampleRate, bands [index].frequency);
                break;
            case LowShelf:
                newCoefficients = dsp::IIR::Coefficients<float>::makeLowShelf (sampleRate, bands [index].frequency, bands [index].quality, bands [index].gain);
                break;
            case BandPass:
                newCoefficients = dsp::IIR::Coefficients<float>::makeBandPass (sampleRate, bands [index].frequency, bands [index].quality);
                break;
            case AllPass:
                newCoefficients = dsp::IIR::Coefficients<float>::makeAllPass (sampleRate, bands [index].frequency, bands [index].quality);
                break;
            case AllPass1st:
                newCoefficients = dsp::IIR::Coefficients<float>::makeFirstOrderAllPass (sampleRate, bands [index].frequency);
                break;
            case Notch:
                newCoefficients = dsp::IIR::Coefficients<float>::makeNotch (sampleRate, bands [index].frequency, bands [index].quality);
                break;
            case Peak:
                newCoefficients = dsp::IIR::Coefficients<float>::makePeakFilter (sampleRate, bands [index].frequency, bands [index].quality, bands [index].gain);
                break;
            case HighShelf:
                newCoefficients = dsp::IIR::Coefficients<float>::makeHighShelf (sampleRate, bands [index].frequency, bands [index].quality, bands [index].gain);
                break;
            case HighPass1st:
                newCoefficients = dsp::IIR::Coefficients<float>::makeFirstOrderHighPass (sampleRate, bands [index].frequency);
                break;
            case HighPass:
                newCoefficients = dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, bands [index].frequency, bands [index].quality);
                break;
            default:
                break;
        }

        if (newCoefficients)
        {
            {
                // minimise lock scope, get<0>() needs to be a compile time constant
                ScopedLock processLock (getCallbackLock()); // getCallbackLock() This returns a critical section that will automatically be locked while the host is calling the processBlock() method.
                if (index == 0)
                    *filter.get<0>().state = *newCoefficients;
                else if (index == 1)
                    *filter.get<1>().state = *newCoefficients;
                else if (index == 2)
                    *filter.get<2>().state = *newCoefficients;
                else if (index == 3)
                    *filter.get<3>().state = *newCoefficients;
                else if (index == 4)
                    *filter.get<4>().state = *newCoefficients;
                else if (index == 5)
                    *filter.get<5>().state = *newCoefficients;
            }
            newCoefficients->getMagnitudeForFrequencyArray (frequencies.data(),
                                                            bands [index].magnitudes.data(),
                                                            frequencies.size(), sampleRate); // Returns the magnitude frequency response of the filter for a given frequency array and sample rate.
        }
        updateBypassedStates();
        updatePlots();
    }
}

void EqualizerAudioProcessor::updatePlots ()
{
    auto gain = filter.get<6>().getGainLinear(); // Returns the current gain as a linear value.
    std::fill (magnitudes.begin(), magnitudes.end(), gain); // 将gain赋值给从begin到end的所有元素

    if (isPositiveAndBelow (soloed, bands.size())) // 判断 0 =< soled < band.size()，如果有soloed
    {
        FloatVectorOperations::multiply (magnitudes.data(), bands [size_t (soloed)].magnitudes.data(), static_cast<int> (magnitudes.size())); // Multiplies the destination values by the source values. magnitudes.data()返回指向magnitudes的指针，最后的int是相乘的数目
    }
    else // 如果没soloed，就依次乘过去
    {
        for (size_t i=0; i < bands.size(); ++i)
            if (bands[i].active) // 且已激活
                FloatVectorOperations::multiply (magnitudes.data(), bands [i].magnitudes.data(), static_cast<int> (magnitudes.size()));
    }
    sendChangeMessage(); // 向所有已注册的linstener发送异步更改消息。
}

//==============================================================================
bool EqualizerAudioProcessor::hasEditor() const
{
    return true;
}

AudioProcessorEditor* EqualizerAudioProcessor::createEditor()
{
    return new EqualizerAudioProcessorEditor (*this);
}

const std::vector<double>& EqualizerAudioProcessor::getMagnitudes ()
{
    return magnitudes;
}

void EqualizerAudioProcessor::createFrequencyPlot (Path& p, const std::vector<double>& mags, const Rectangle<int> bounds, float pixelsPerDouble) // 此函数绘制控制滤波器的频率曲线
{
    p.startNewSubPath (bounds.getX(), roundToInt (bounds.getCentreY() - pixelsPerDouble * std::log (mags [0]) / std::log (2))); // Begins a new subpath with a given starting position. 设置一个新起点，否则会有跳变
    const double xFactor = static_cast<double> (bounds.getWidth()) / frequencies.size();
    
    for (size_t i=1; i < frequencies.size(); ++i)
    {   // Adds a line from the shape's last position to a new end-point. This will connect the end-point of the last line or curve that was added to a new point, using a straight line. 通过循环以直线连接各个频率点
        p.lineTo (roundToInt (bounds.getX() + i * xFactor),
                  roundToInt (bounds.getCentreY() - pixelsPerDouble * std::log (mags [i]) / std::log (2)));
    }
}

void EqualizerAudioProcessor::createAnalyserPlot (Path& p, const Rectangle<int> bounds, float minFreq, bool input) // 此函数绘制输入输出的频率曲线
{
    if (input)
        inputAnalyser.createPath (p, bounds.toFloat(), minFreq);
    else
        outputAnalyser.createPath (p, bounds.toFloat(), minFreq);
}

bool EqualizerAudioProcessor::checkForNewAnalyserData()
{
    return inputAnalyser.checkForNewData() || outputAnalyser.checkForNewData();
}

//==============================================================================
void EqualizerAudioProcessor::getStateInformation (MemoryBlock& destData) // 模版有此函数
{
    auto editor = parameters.state.getOrCreateChildWithName (IDs::editor, nullptr);
    editor.setProperty (IDs::sizeX, editorSize.x, nullptr);
    editor.setProperty (IDs::sizeY, editorSize.y, nullptr);

    MemoryOutputStream stream(destData, false);
    parameters.state.writeToStream (stream);
}

void EqualizerAudioProcessor::setStateInformation (const void* data, int sizeInBytes) // 模版有此函数
{
    ValueTree tree = ValueTree::readFromData (data, size_t (sizeInBytes));
    if (tree.isValid())
    {
        parameters.state = tree;

        auto editor = parameters.state.getChildWithName (IDs::editor);
        if (editor.isValid())
        {
            editorSize.setX (editor.getProperty (IDs::sizeX, 900));
            editorSize.setY (editor.getProperty (IDs::sizeY, 500));
            if (auto* activeEditor = getActiveEditor())
                activeEditor->setSize (editorSize.x, editorSize.y);
        }
    }
}

Point<int> EqualizerAudioProcessor::getSavedSize() const
{
    return editorSize;
}

void EqualizerAudioProcessor::setSavedSize (const Point<int>& size)
{
    editorSize = size;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EqualizerAudioProcessor();
}
