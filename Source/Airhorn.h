/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             PlayingSoundFilesTutorial
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Plays audio files.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2019, linux_make

 type:             Component
 mainClass:        MainContentComponent

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once
#define MAX_AREA 10000000.0

//==============================================================================
class MainContentComponent   : public juce::AudioAppComponent,
                               public juce::ChangeListener
{
public:
    MainContentComponent()
        : state (Stopped)
    {
        setSize (500, 500);
        
        
        // Airhorn image as button to trigger sample. Button scales with application window.
        myImageButton.setBounds (0, 0, getWidth(), getHeight());
        Image airhornButton = ImageCache::getFromMemory(BinaryData::airhorn_png, BinaryData::airhorn_pngSize);
        myImageButton.setImages(false, true, false, airhornButton, 1.0f, {}, Image(), 1.0f, {}, Image(), 1.0f, {});
        myImageButton.onClick = [this] { myImageButtonClicked(); };
        myImageButton.setEnabled (true);
        addAndMakeVisible(&myImageButton);
        
        area = getWidth()*getHeight(); //Area of application used to scale gain
        
        formatManager.registerBasicFormats();
        transportSource.addChangeListener (this);
        formatManager.getWildcardForAllFormats();
        
        auto* reader = formatManager.createReaderFor(std::make_unique<juce::MemoryInputStream>(BinaryData::airhorn_wav, BinaryData::airhorn_wavSize, false));
        std::unique_ptr<juce::AudioFormatReaderSource> newSource (new juce::AudioFormatReaderSource (reader, true));
        transportSource.setSource (newSource.get(), 0, nullptr, reader->sampleRate);
        readerSource.reset (newSource.release());
        
        setAudioChannels (0, 2);
    }

    ~MainContentComponent() override
    {
        shutdownAudio();
    }

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        transportSource.prepareToPlay (samplesPerBlockExpected, sampleRate);
    }

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        if (readerSource.get() == nullptr)
        {
            bufferToFill.clearActiveBufferRegion();
            return;
        }

        transportSource.getNextAudioBlock (bufferToFill);
    }

    void releaseResources() override
    {
        transportSource.releaseResources();
    }

    void resized() override
    {
        myImageButton.setBounds(0, 0, getWidth(), getHeight());
        area = getWidth()*getHeight();
        transportSource.setGain(area / MAX_AREA); //Scale gain based on application window size
    }
    

    void changeListenerCallback (juce::ChangeBroadcaster* source) override
    {
        if (source == &transportSource)
        {
            if (transportSource.isPlaying())
                changeState (Playing);
        }
    }

private:
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Stopping
    };

    void changeState (TransportState newState)
    {
        if (state != newState)
        {
            state = newState;

            switch (state)                              // Defines playback states
            {
                case Stopped:
                    transportSource.setPosition (0.0);
                    break;

                case Starting:
                    transportSource.stop();
                    transportSource.setPosition (0.0);
                    transportSource.start();
                    break;

                case Playing:
                    break;

                case Stopping:
                    transportSource.stop();
                    break;
            }
        }
    }

    void myImageButtonClicked()
    {
        if (state == Stopped)
            changeState (Starting);
        else if (state == Playing)
            changeState (Starting);
    }

    //==========================================================================
    juce::ImageButton myImageButton;
    float area;
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    TransportState state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
