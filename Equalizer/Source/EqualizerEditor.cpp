/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "Analyser.h"
#include "EqualizerProcessor.h"
#include "SocialButtons.h"
#include "EqualizerEditor.h"

static int   clickRadius = 4;
static float maxDB       = 24.0f;

//==============================================================================
EqualizerAudioProcessorEditor::EqualizerAudioProcessorEditor (EqualizerAudioProcessor& p)
  : AudioProcessorEditor (&p), processor (p),
    output (Slider::RotaryHorizontalVerticalDrag, Slider::TextBoxBelow)
{
    tooltipWindow->setMillisecondsBeforeTipAppears (1000); // Changes the time before the tip appears, default value is 700.

    addAndMakeVisible (socialButtons);

    for (size_t i=0; i < processor.getNumBands(); ++i)
    {
        auto* bandEditor = bandEditors.add (new BandEditor (i, processor));
        addAndMakeVisible (bandEditor);
    }

    frame.setText (TRANS ("Output")); // Changes the text that's shown at the top of the component.
    frame.setTextLabelPosition (Justification::centred); // Sets the positioning of the text label.
    addAndMakeVisible (frame);
    addAndMakeVisible (output);
    attachments.add (new AudioProcessorValueTreeState::SliderAttachment (processor.getPluginState(), EqualizerAudioProcessor::paramOutput, output)); // An object of this class maintains a connection between a Slider and a parameter in an AudioProcessorValueTreeState. Three parameters:  1.AudioProcessorValueTreeState & tateToControl, 2.const String & parameterID, 3.Slider & sliderToControl
    output.setTooltip (TRANS ("Overall Gain"));

    auto size = processor.getSavedSize();
    setResizable (true, true); // Marks the host's editor window as resizable. 1.bool allowHostToResize, 2.bool useBottomRightCornerResizer
    setSize (size.x, size.y); // Changes the size of the component. 类似初始值
    setResizeLimits (800, 450, 2990, 1800); // This sets the maximum and minimum sizes for the window. 图形窗口可以拉伸的范围

    updateFrequencyResponses();

#ifdef JUCE_OPENGL
    openGLContext.attachTo (*getTopLevelComponent()); // Attaches the context to a target component.
    // Returns the highest-level component which contains this one or its parents.
#endif

    processor.addChangeListener (this); // Registers a listener to receive change callbacks from this broadcaster.

    startTimerHz (30); // Starts the timer with an interval specified in Hertz. 以指定时间间隔启动计时器
}

EqualizerAudioProcessorEditor::~EqualizerAudioProcessorEditor()
{
    PopupMenu::dismissAllActiveMenus(); // Closes any menus that are currently open.

    processor.removeChangeListener (this); // Unregisters a listener from the list.
#ifdef JUCE_OPENGL
    openGLContext.detach(); // Detaches the context from its target component and deletes any native resources.
#endif
}

//==============================================================================
void EqualizerAudioProcessorEditor::paint (Graphics& g)
{
    const Colour inputColour = Colours::greenyellow;
    const Colour outputColour = Colours::indianred;

    Graphics::ScopedSaveState state (g); // Uses RAII to save and restore the state of a graphics context.

    g.fillAll(Colours::black);
    //g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId)); // Fills the context's entire clip region with a given colour.
    // Finds the appropriate look-and-feel to use for this component. LookAndFeel& Component::getLookAndFeel () const
    // Looks for a colour that has been registered with the given colour ID number. Colour LookAndFeel::findColour (int colourId) const
    auto logo = ImageCache::getFromMemory (DNVAudioData::LogoDNV_png, DNVAudioData::LogoDNV_pngSize);
    g.drawImage (logo, brandingFrame.toFloat(), RectanglePlacement (RectanglePlacement::fillDestination)); // Draws an image, having applied an affine transform to it.
    // Casts this rectangle to a Rectangle<float>. brandingFrame.toFloat()
    // Creates a RectanglePlacement object using a combination of flags from the Flags enum. Defines the method used to position some kind of rectangular object within a rectangular viewport.

    g.setFont (12.0f); // Changes the size of the currently-selected font.
    g.setColour (Colours::silver);
    g.drawRoundedRectangle (plotFrame.toFloat(), 5, 2); // Uses the current colour or brush to draw the outline of a rectangle with rounded corners.
    
    // 此循环绘制横坐标，共10个
    for (int i=0; i <10; ++i)
    {
        g.setColour (Colours::silver.withAlpha(0.3f)); // Alpha is transparency
        auto x = plotFrame.getX() + plotFrame.getWidth() * i * 0.1f;
        
        // 绘制网格竖线
        //if (i > 0) g.drawVerticalLine (roundToInt (x), plotFrame.getY(), plotFrame.getBottom());

        g.setColour (Colours::silver);
        auto freq = getFrequencyForPosition (i * 0.1f);
        g.drawFittedText ((freq < 1000) ? String (freq) + " Hz" : String (freq / 1000, 1) + " kHz",
                          roundToInt (x + 3), plotFrame.getBottom() - 18, 50, 15, Justification::left, 1);
    }

    // 以下3行代码绘制网格横线
    //g.setColour (Colours::silver.withAlpha (0.3f));
    //g.drawHorizontalLine (roundToInt (plotFrame.getY() + 0.25 * plotFrame.getHeight()), plotFrame.getX(), plotFrame.getRight()); // Draws a horizontal line of pixels at a given y position.
    //g.drawHorizontalLine (roundToInt (plotFrame.getY() + 0.75 * plotFrame.getHeight()), plotFrame.getX(), plotFrame.getRight());

    // 以下5行代码绘制纵坐标
    g.setColour (Colours::silver);
    g.drawFittedText (String (maxDB) + " dB", plotFrame.getX() + 3, plotFrame.getY() + 2, 50, 14, Justification::left, 1);
    g.drawFittedText (String (maxDB / 2) + " dB", plotFrame.getX() + 3, roundToInt (plotFrame.getY() + 2 + 0.25 * plotFrame.getHeight()), 50, 14, Justification::left, 1);
    g.drawFittedText (" 0 dB", plotFrame.getX() + 3, roundToInt (plotFrame.getY() + 2 + 0.5  * plotFrame.getHeight()), 50, 14, Justification::left, 1);
    g.drawFittedText (String (- maxDB / 2) + " dB", plotFrame.getX() + 3, roundToInt (plotFrame.getY() + 2 + 0.75 * plotFrame.getHeight()), 50, 14, Justification::left, 1);

    g.reduceClipRegion (plotFrame); // Intersects the current clipping region with another region. 裁剪超出显示区域的部分

    g.setFont (20.0f); // 设置Inputh和Output字体大小
    processor.createAnalyserPlot (analyserPath, plotFrame, 20.0f, true); // Draw Input.
    g.setColour (inputColour);
    g.drawFittedText ("Input", plotFrame.reduced (8,10), Justification::topRight, 1);
    g.strokePath (analyserPath, PathStrokeType (1.0)); // Draws a path's outline using the currently selected colour or brush.
    processor.createAnalyserPlot (analyserPath, plotFrame, 20.0f, false); // Draw Output.
    g.setColour (outputColour);
    g.drawFittedText ("Output", plotFrame.reduced (8, 28), Justification::topRight, 1);
    g.strokePath (analyserPath, PathStrokeType (1.0)); // Draws a path's outline using the currently selected colour or brush.

    // 此循环画band对应的竖线
    for (size_t i=0; i < processor.getNumBands(); ++i)
    {
        auto* bandEditor = bandEditors.getUnchecked (int (i));
        auto* band = processor.getBand (i);

        g.setColour (band->active ? band->colour : band->colour.withAlpha (0.3f));
        g.strokePath (bandEditor->frequencyResponse, PathStrokeType (1.0));
        g.setColour (draggingBand == int (i) ? band->colour : band->colour.withAlpha (0.3f));
        auto x = roundToInt (plotFrame.getX() + plotFrame.getWidth() * getPositionForFrequency (float (band->frequency)));
        auto y = roundToInt (getPositionForGain (float (band->gain), plotFrame.getY(), plotFrame.getBottom()));
        g.drawVerticalLine (x, plotFrame.getY(), y - 5);
        g.drawVerticalLine (x, y + 5, plotFrame.getBottom());
        g.fillEllipse (x - 3, y - 3, 6, 6);
    }
    
    // 以下2行代码绘制汇总的白色频率响应
    g.setColour (Colours::silver);
    g.strokePath (frequencyResponse, PathStrokeType (2.0)); // Draws a path's outline using the currently selected colour or brush.
}

void EqualizerAudioProcessorEditor::resized()
{
    processor.setSavedSize ({ getWidth(), getHeight() });
    plotFrame = getLocalBounds().reduced (3, 3); // 控制插件外围边框大小，值越大边框越大

    socialButtons.setBounds (plotFrame.removeFromBottom (35)); // 控制最下面SocialButtons和Text的大小，值越大区域越大

    auto bandSpace = plotFrame.removeFromBottom (getHeight() / 2); // 划分一半高度的区域绘制filter band
    auto width = roundToInt (bandSpace.getWidth()) / (bandEditors.size() + 1); // 设置每个filter的宽度，加1是为了留出空给Output
    //此循环绘制filter band
    for (auto* bandEditor : bandEditors)
        bandEditor->setBounds (bandSpace.removeFromLeft (width));

    frame.setBounds (bandSpace.removeFromTop (bandSpace.getHeight() / 2)); // 划分一半高度的区域绘制Output
    output.setBounds (frame.getBounds().reduced (8)); // 控制Output区域大小，值越大区域越小

    plotFrame.reduce (3, 3); // 控制绘图区域大小，值越大区域越小
    brandingFrame = bandSpace.reduced (3); // 控制右下角Logo大小，值越大Logo越小

    updateFrequencyResponses();
}

void EqualizerAudioProcessorEditor::changeListenerCallback (ChangeBroadcaster* sender)
{
    ignoreUnused (sender);
    updateFrequencyResponses();
    repaint(); // Marks the whole component as needing to be redrawn.
}

void EqualizerAudioProcessorEditor::timerCallback()
{
    if (processor.checkForNewAnalyserData())
        repaint (plotFrame); // Marks a subsection of this component as needing to be redrawn.
}

void EqualizerAudioProcessorEditor::mouseDown (const MouseEvent& e)
{
    // 1.Checks whether the user is trying to launch a pop-up menu. 2.Returns true if this coordinate is inside the rectangle.
    if (! e.mods.isPopupMenu() || ! plotFrame.contains (e.x, e.y))
        return;

    for (int i=0; i < bandEditors.size(); ++i)
    {
        if (auto* band = processor.getBand (size_t (i)))
        {
            if (std::abs (plotFrame.getX() + getPositionForFrequency (int (band->frequency)) * plotFrame.getWidth()
                          - e.position.getX()) < clickRadius)
            {
                contextMenu.clear(); // Resets the menu, removing all its items.
                const auto& names = EqualizerAudioProcessor::getFilterTypeNames();
                
                // 将band type加入menu
                for (int t=0; t < names.size(); ++t)
                    contextMenu.addItem (t + 1, names [t], true, band->type == t); // Appends a new text item for this menu to show.
                
                // Runs the menu asynchronously, with a user-provided callback that will receive the result.
                contextMenu.showMenuAsync (PopupMenu::Options()
                                           .withTargetComponent (this)
                                           .withTargetScreenArea ({e.getScreenX(), e.getScreenY(), 1, 1})
                                           , [this, i](int selected)
                                           {
                                               if (selected > 0)
                                                   bandEditors.getUnchecked (i)->setType (selected - 1);
                                           });
                return;
            }
        }
    }
}

void EqualizerAudioProcessorEditor::mouseMove (const MouseEvent& e)
{
    if (plotFrame.contains (e.x, e.y))
    {
        for (int i=0; i < bandEditors.size(); ++i)
        {
            if (auto* band = processor.getBand (size_t (i)))
            {
                auto pos = plotFrame.getX() + getPositionForFrequency (float (band->frequency)) * plotFrame.getWidth();

                if (std::abs (pos - e.position.getX()) < clickRadius) // 如果横坐标在竖线范围内
                {
                    if (std::abs (getPositionForGain (float (band->gain), plotFrame.getY(), plotFrame.getBottom())
                                  - e.position.getY()) < clickRadius) // 如果纵坐标也在竖线范围内
                    {
                        draggingGain = processor.getPluginState().getParameter (processor.getGainParamName (size_t (i))); // 从光标的到gain值
                        setMouseCursor (MouseCursor (MouseCursor::UpDownLeftRightResizeCursor)); // 更改鼠标在此组件上时使用的鼠标光标形状，改为十字型
                    }
                    else
                    {
                        setMouseCursor (MouseCursor (MouseCursor::LeftRightResizeCursor)); // 更改鼠标的光标形状为一字型
                    }

                    if (i != draggingBand)
                    {
                        draggingBand = i;
                        repaint (plotFrame);
                    }
                    return;
                }
            }
        }
    }
    draggingBand = -1;
    draggingGain = false;
    setMouseCursor (MouseCursor (MouseCursor::NormalCursor)); // 光标设为标准形状
    repaint (plotFrame);
}

void EqualizerAudioProcessorEditor::mouseDrag (const MouseEvent& e)
{
    if (isPositiveAndBelow (draggingBand, bandEditors.size())) // Returns true if bandEditors.size() > draggingBand >= 0.
    {
        auto pos = (e.position.getX() - plotFrame.getX()) / plotFrame.getWidth();
        bandEditors [draggingBand]->setFrequency (getFrequencyForPosition (pos));
        if (draggingGain)
            bandEditors [draggingBand]->setGain (getGainForPosition (e.position.getY(), plotFrame.getY(), plotFrame.getBottom()));
    }
}

void EqualizerAudioProcessorEditor::mouseDoubleClick (const MouseEvent& e) // 双击竖线激活filter?
{
    if (plotFrame.contains (e.x, e.y))
    {
        for (size_t i=0; i < size_t (bandEditors.size()); ++i)
        {
            if (auto* band = processor.getBand (i))
            {
                if (std::abs (plotFrame.getX() + getPositionForFrequency (float (band->frequency)) * plotFrame.getWidth()
                              - e.position.getX()) < clickRadius)
                {
                    if (auto* param = processor.getPluginState().getParameter (processor.getActiveParamName (i)))
                        param->setValueNotifyingHost (param->getValue() < 0.5f ? 1.0f : 0.0f); // A processor should call this when it needs to change one of its parameters. Parameter is new value.
                }
            }
        }
    }
}

void EqualizerAudioProcessorEditor::updateFrequencyResponses ()
{
    auto pixelsPerDouble = 2.0f * plotFrame.getHeight() / Decibels::decibelsToGain (maxDB); // 2倍的频谱图高度比maxGain，得到单位高度的gain

    for (int i=0; i < bandEditors.size(); ++i)
    {
        auto* bandEditor = bandEditors.getUnchecked (i); // Returns a pointer to the object at this index in the array, without checking whether the index is in-range.

        if (auto* band = processor.getBand (size_t (i)))
        {
            bandEditor->updateControls (band->type); // 控制不同filter的3个旋钮
            bandEditor->frequencyResponse.clear(); // Removes all lines and curves, resetting the path completely.
            processor.createFrequencyPlot (bandEditor->frequencyResponse, band->magnitudes, plotFrame.withX (plotFrame.getX() + 1), pixelsPerDouble);
        }
        bandEditor->updateSoloState (processor.getBandSolo (i));
    }
    frequencyResponse.clear(); // Removes all lines and curves, resetting the path completely.
    processor.createFrequencyPlot (frequencyResponse, processor.getMagnitudes(), plotFrame, pixelsPerDouble);
}

float EqualizerAudioProcessorEditor::getPositionForFrequency (float freq)
{
    return (std::log (freq / 20.0f) / std::log (2.0f)) / 10.0f;
}

float EqualizerAudioProcessorEditor::getFrequencyForPosition (float pos)
{
    return 20.0f * std::pow (2.0f, pos * 10.0f);
}

float EqualizerAudioProcessorEditor::getPositionForGain (float gain, float top, float bottom)
{
    return jmap (Decibels::gainToDecibels (gain, -maxDB), -maxDB, maxDB, bottom, top);
}

float EqualizerAudioProcessorEditor::getGainForPosition (float pos, float top, float bottom)
{
    return Decibels::decibelsToGain (jmap (pos, bottom, top, -maxDB, maxDB), -maxDB);
}


//==============================================================================
//这个类用来绘制band里的具体细节
EqualizerAudioProcessorEditor::BandEditor::BandEditor (size_t i, EqualizerAudioProcessor& p)
  : index (i),
    processor (p),
    frequency (Slider::RotaryHorizontalVerticalDrag, Slider::TextBoxBelow), // 设置旋钮和注解的样式
    quality   (Slider::RotaryHorizontalVerticalDrag, Slider::TextBoxBelow),
    gain      (Slider::RotaryHorizontalVerticalDrag, Slider::TextBoxBelow),
    solo      (TRANS ("S")),
    activate  (TRANS ("A"))
{
    frame.setText (processor.getBandName (index));
    frame.setTextLabelPosition (Justification::centred);
    frame.setColour (GroupComponent::textColourId, processor.getBandColour (index));
    frame.setColour (GroupComponent::outlineColourId, processor.getBandColour (index));
    addAndMakeVisible (frame);

    if (auto* choiceParameter = dynamic_cast<AudioParameterChoice*>(processor.getPluginState().getParameter (processor.getTypeParamName (index))))
        filterType.addItemList (choiceParameter->choices, 1);

    addAndMakeVisible (filterType);
    boxAttachments.add (new AudioProcessorValueTreeState::ComboBoxAttachment (processor.getPluginState(), processor.getTypeParamName (index), filterType)); // An object of this class maintains a connection between a Slider and a parameter in an AudioProcessorValueTreeState.

    addAndMakeVisible (frequency);
    attachments.add (new AudioProcessorValueTreeState::SliderAttachment (processor.getPluginState(), processor.getFrequencyParamName (index), frequency)); // An object of this class maintains a connection between a Slider and a parameter in an AudioProcessorValueTreeState.

    frequency.setSkewFactorFromMidPoint (1000.0); // 设置一个倾斜因子来更改值的分布方式,指定应该出现在滑块可见范围中心的滑块值
    frequency.setTooltip (TRANS ("Filter's frequency"));

    addAndMakeVisible (quality);
    attachments.add (new AudioProcessorValueTreeState::SliderAttachment (processor.getPluginState(), processor.getQualityParamName (index), quality)); // An object of this class maintains a connection between a Slider and a parameter in an AudioProcessorValueTreeState.

    quality.setSkewFactorFromMidPoint (1.0); // // 设置一个倾斜因子来更改值的分布方式,指定应该出现在滑块可见范围中心的滑块值
    quality.setTooltip (TRANS ("Filter's steepness (Quality)"));

    addAndMakeVisible (gain);
    attachments.add (new AudioProcessorValueTreeState::SliderAttachment (processor.getPluginState(), processor.getGainParamName (index), gain)); // An object of this class maintains a connection between a Slider and a parameter in an AudioProcessorValueTreeState.
    gain.setSkewFactorFromMidPoint (1.0); // // 设置一个倾斜因子来更改值的分布方式,指定应该出现在滑块可见范围中心的滑块值
    gain.setTooltip (TRANS ("Filter's gain"));

    solo.setClickingTogglesState (true); // 这将告诉按钮在单击按钮时自动按下开关
    solo.addListener (this); // 注意这里与buttonAttachments.add的区别，估计Solo是默认关闭而Activate是默认打开而引起的区别
    solo.setColour (TextButton::buttonOnColourId, Colours::yellow); // 设置按下开关后按钮的颜色
    addAndMakeVisible (solo);
    solo.setTooltip (TRANS ("Listen only through this filter (solo)"));

    activate.setClickingTogglesState (true); // 这将告诉按钮在单击按钮时自动按下开关。
    activate.setColour (TextButton::buttonOnColourId, Colours::green); // 设置按下开关后按钮的颜色
    buttonAttachments.add (new AudioProcessorValueTreeState::ButtonAttachment (processor.getPluginState(), processor.getActiveParamName (index), activate));
    addAndMakeVisible (activate);
    activate.setTooltip (TRANS ("Activate or deactivate this filter"));
}

void EqualizerAudioProcessorEditor::BandEditor::resized ()
{
    auto bounds = getLocalBounds(); // Returns the component's bounds, relative to its own origin.
    frame.setBounds (bounds);

    bounds.reduce (10, 20); // 设置滤波器选项的位置

    filterType.setBounds (bounds.removeFromTop (20)); // 设置滤波器选项的大小

    auto freqBounds = bounds.removeFromBottom (bounds.getHeight() * 2 / 3); // 频率选项占2/3的高度
    frequency.setBounds (freqBounds.withTop (freqBounds.getY() + 10)); // 设置频率旋钮大小

    auto buttons = freqBounds.reduced (5).withHeight (20); // 设置S和A的位置，reudced是离边框的距离，withHegiht是高
    solo.setBounds (buttons.removeFromLeft (20)); // 设置S的宽度
    activate.setBounds (buttons.removeFromRight (20)); // 设置A的宽度

    quality.setBounds (bounds.removeFromLeft (bounds.getWidth() / 2)); // 设置quality旋钮的大小
    gain.setBounds (bounds); // 设置gain旋钮的大小
}

void EqualizerAudioProcessorEditor::BandEditor::updateControls (EqualizerAudioProcessor::FilterType type)
{
    switch (type) {
        case EqualizerAudioProcessor::LowPass:
            frequency.setEnabled (true); quality.setEnabled (true); gain.setEnabled (false);
            break;
        case EqualizerAudioProcessor::LowPass1st:
            frequency.setEnabled (true); quality.setEnabled (false); gain.setEnabled (false);
            break;
        case EqualizerAudioProcessor::LowShelf:
            frequency.setEnabled (true); quality.setEnabled (false); gain.setEnabled (true);
            break;
        case EqualizerAudioProcessor::BandPass:
            frequency.setEnabled (true); quality.setEnabled (true); gain.setEnabled (false);
            break;
        case EqualizerAudioProcessor::AllPass:
            frequency.setEnabled (true); quality.setEnabled (false); gain.setEnabled (false);
            break;
        case EqualizerAudioProcessor::AllPass1st:
            frequency.setEnabled (true); quality.setEnabled (false); gain.setEnabled (false);
            break;
        case EqualizerAudioProcessor::Notch:
            frequency.setEnabled (true); quality.setEnabled (true); gain.setEnabled (false);
            break;
        case EqualizerAudioProcessor::Peak:
            frequency.setEnabled (true); quality.setEnabled (true); gain.setEnabled (true);
            break;
        case EqualizerAudioProcessor::HighShelf:
            frequency.setEnabled (true); quality.setEnabled (true); gain.setEnabled (true);
            break;
        case EqualizerAudioProcessor::HighPass1st:
            frequency.setEnabled (true); quality.setEnabled (false); gain.setEnabled (false);
            break;
        case EqualizerAudioProcessor::HighPass:
            frequency.setEnabled (true); quality.setEnabled (true); gain.setEnabled (false);
            break;
        default:
            frequency.setEnabled (true);
            quality.setEnabled (true);
            gain.setEnabled (true);
            break;
    }
}

void EqualizerAudioProcessorEditor::BandEditor::updateSoloState (bool isSolo)
{
    solo.setToggleState (isSolo, dontSendNotification);
}

void EqualizerAudioProcessorEditor::BandEditor::setFrequency (float freq)
{
    frequency.setValue (freq, sendNotification);
}

void EqualizerAudioProcessorEditor::BandEditor::setGain (float gainToUse)
{
    gain.setValue (gainToUse, sendNotification);
}

void EqualizerAudioProcessorEditor::BandEditor::setType (int type)
{
    filterType.setSelectedId (type + 1, sendNotification);
}

void EqualizerAudioProcessorEditor::BandEditor::buttonClicked (Button* b)
{
    if (b == &solo) {
        processor.setBandSolo (solo.getToggleState() ? int (index) : -1);
    }
}
