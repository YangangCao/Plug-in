#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
template<typename Type>
class Analyser : public Thread
{
public:
    Analyser() : Thread ("Equaliser-Analyser") // Create a thread named "Equalizer-Analyser"
    {
        averager.clear(); // Clears all the samples in all channels.
    }

    virtual ~Analyser() = default;

    void addAudioData (const AudioBuffer<Type>& buffer, int startChannel, int numChannels)
    {
        if (abstractFifo.getFreeSpace() < buffer.getNumSamples()) // Returns the number of items that can currently be added to the buffer without it overflowing.
            return;

        int start1, block1, start2, block2;
        abstractFifo.prepareToWrite (buffer.getNumSamples(), start1, block1, start2, block2); // Returns the location within the buffer at which an incoming block of data should be written. After calling this method, if you choose to write your data into the blocks returned, you must call finishedWrite() to tell the FIFO how much data you actually added.
        audioFifo.copyFrom (0, start1, buffer.getReadPointer (startChannel), block1); //Copies samples from an array of floats into one of the channels.
        if (block2 > 0)
            audioFifo.copyFrom (0, start2, buffer.getReadPointer (startChannel, block1), block2);

        for (int channel = startChannel + 1; channel < startChannel + numChannels; ++channel)
        {
            if (block1 > 0) audioFifo.addFrom (0, start1, buffer.getReadPointer (channel), block1); // Adds samples from an array of floats to one of the channels.
            if (block2 > 0) audioFifo.addFrom (0, start2, buffer.getReadPointer (channel, block1), block2);
        }
        abstractFifo.finishedWrite (block1 + block2); // Called after writing from the FIFO, to indicate that this many items have been added.
        waitForData.signal(); // Wakes up any threads that are currently waiting on this object.
    }

    void setupAnalyser (int audioFifoSize, Type sampleRateToUse)
    {
        sampleRate = sampleRateToUse;
        audioFifo.setSize (1, audioFifoSize); // Changes the buffer's size or number of channels.
        abstractFifo.setTotalSize (audioFifoSize); // Changes the buffer's total size.

        startThread (5); // Starts the thread with a given priority, 0 = lowest, 10 = highest.
    }

    void run() override
    {
        while (! threadShouldExit()) // Checks whether the thread has been told to stop running.
        {
            if (abstractFifo.getNumReady() >= fft.getSize()) // Returns the number of items that can currently be read from the buffer.
                // Returns the number of data points that this FFT was created to work with
            {
                fftBuffer.clear(); // Clears all the samples in all channels.

                int start1, block1, start2, block2;
                abstractFifo.prepareToRead (fft.getSize(), start1, block1, start2, block2); // Returns the location within the buffer from which the next block of data should be read. After calling this method, if you choose to read the data, you must call finishedRead() to tell the FIFO how much data you have consumed.
                if (block1 > 0) fftBuffer.copyFrom (0, 0, audioFifo.getReadPointer (0, start1), block1); //Copies samples from an array of floats into one of the channels.
                if (block2 > 0) fftBuffer.copyFrom (0, block1, audioFifo.getReadPointer (0, start2), block2);
                abstractFifo.finishedRead ((block1 + block2) / 2);

                windowing.multiplyWithWindowingTable (fftBuffer.getWritePointer (0), size_t (fft.getSize())); // Multiplies the content of a buffer with the given window.
                fft.performFrequencyOnlyForwardTransform (fftBuffer.getWritePointer (0)); // Takes an array and simply transforms it to the magnitude frequency response spectrum.

                ScopedLock lockedForWriting (pathCreationLock); // Automatically locks and unlocks a CriticalSection object. You can use a ScopedLock as a local variable to provide RAII-based locking of a CriticalSection.
                averager.addFrom (0, 0, averager.getReadPointer (averagerPtr), averager.getNumSamples(), -1.0f);
                averager.copyFrom (averagerPtr, 0, fftBuffer.getReadPointer (0), averager.getNumSamples(), 1.0f / (averager.getNumSamples() * (averager.getNumChannels() - 1)));
                averager.addFrom (0, 0, averager.getReadPointer (averagerPtr), averager.getNumSamples());
                if (++averagerPtr == averager.getNumChannels()) averagerPtr = 1;

                newDataAvailable = true;
            }

            if (abstractFifo.getNumReady() < fft.getSize())
                waitForData.wait (100); // Suspends the calling thread until the event has been signalled.
        }
    }

    void createPath (Path& p, const Rectangle<float> bounds, float minFreq)
    {
        p.clear(); // Removes all lines and curves, resetting the path completely.
        p.preallocateSpace (8 + averager.getNumSamples() * 3); // Preallocates enough space for adding the given number of coordinates to the path.

        ScopedLock lockedForReading (pathCreationLock);
        const auto* fftData = averager.getReadPointer (0);
        const auto  factor  = bounds.getWidth() / 10.0f; // Returns the height of the rectangle.

        p.startNewSubPath (bounds.getX() + factor * indexToX (0, minFreq), binToY (fftData [0], bounds)); // Begins a new subpath with a given starting position.
        for (int i = 0; i < averager.getNumSamples(); ++i)
            p.lineTo (bounds.getX() + factor * indexToX (i, minFreq), binToY (fftData [i], bounds)); // Adds a line from the shape's last position to a new end-point. void Path::lineTo (float endX, float endY)
    }

    bool checkForNewData()
    {
        auto available = newDataAvailable;
        newDataAvailable = false;
        return available;
    }

private:

    inline float indexToX (float index, float minFreq) const
    {
        const auto freq = (sampleRate * index) / fft.getSize();
        return (freq > 0.01f) ? std::log (freq / minFreq) / std::log (2.0f) : 0.0f;
    }

    inline float binToY (float bin, const Rectangle<float> bounds) const
    {
        const float infinity = -80.0f;
        return jmap (Decibels::gainToDecibels (bin, infinity),
                     infinity, 0.0f, bounds.getBottom(), bounds.getY());
    }

    WaitableEvent waitForData;
    CriticalSection pathCreationLock;

    Type sampleRate {};

    dsp::FFT fft                           { 12 };
    dsp::WindowingFunction<Type> windowing { size_t (fft.getSize()), dsp::WindowingFunction<Type>::kaiser, true, 4 };
    AudioBuffer<float> fftBuffer           { 1, fft.getSize() * 2 };

    AudioBuffer<float> averager            { 5, fft.getSize() / 2 };
    int averagerPtr = 1;

    AbstractFifo abstractFifo              { 48000 };
    AudioBuffer<Type> audioFifo;

    bool newDataAvailable = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Analyser)
};
