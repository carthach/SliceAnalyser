/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "GenAudioComponent.h"
#include "TargetAudioComponent.h"
#include "DataComponent.h"
#include "Box2DComponent.h"
#include "Visualisation.h"

class AudioTab : public Component
{
public:
    ApplicationProperties applicationProperties;
    
    AudioTab(AudioDeviceManager* deviceManager, DataComponent* dataComponent) : targetAudioComponent(deviceManager), genAudioComponent(deviceManager, &targetAudioComponent, dataComponent)
    {
        addAndMakeVisible(targetAudioComponent);
        addAndMakeVisible(genAudioComponent);

    }
    
    void resized() override
    {
        // This is called when the MainContentComponent is resized.
        // If you add any child components, this is where you should
        // update their positions.
        
        Rectangle<int> bottomHalf = getLocalBounds();
        Rectangle<int> topHalf = bottomHalf.removeFromTop(bottomHalf.getHeight()/2);
        targetAudioComponent.setBounds(topHalf);
        genAudioComponent.setBounds(bottomHalf);
    }
    
private:
    TargetAudioComponent targetAudioComponent;
    GenAudioComponent genAudioComponent;    
};

//==============================================================================

//class TableTab : public Component
//{
//public:
//    
//    TableTab()
//    {
//        addAndMakeVisible(tableComponent);
//        
//    }
//    
//    void resized() override
//    {
//        // This is called when the MainContentComponent is resized.
//        // If you add any child components, this is where you should
//        // update their positions.
//        
//        tableComponent.setBounds(getLocalBounds());
//    }
//    
//private:
//    TableDemoComponent tableComponent;
//};

//==============================================================================
class MainTab  : public TabbedComponent
{
public:
    MainTab (AudioDeviceManager* deviceManager)
    : TabbedComponent (TabbedButtonBar::TabsAtTop)
    {
        //Need to do it like this so that the Audio gets a pointer to the dataComponent
        DataComponent* dataComponent = new DataComponent();
        addTab ("Audio", Colours::grey, new AudioTab(deviceManager, (DataComponent* )dataComponent), true);
        addTab ("Data",  Colours::grey, dataComponent, true);
        addTab ("Explorer",  Colours::grey, new Visualisation(), true);
        addTab ("Box2D",  Colours::grey, new Box2DDemo(), true);


//        addTab ("Table",            Colours::grey, new TableTab(),           true);            
    }
};

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainContentComponent   : public AudioAppComponent
{
public:
    //==============================================================================
    MainContentComponent() : mainTab(&deviceManager)
    {
        setSize (1280, 1024);
  

        // specify the number of input and output channels that we want to open
        setAudioChannels (2, 2);
        
        addAndMakeVisible(mainTab);

//        setLookAndFeel(&lookAndFeel);
        

    }

    ~MainContentComponent()
    {
        essentia::shutdown();
        shutdownAudio();
    }

    //=======================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        // This function will be called when the audio device is started, or when
        // its settings (i.e. sample rate, block size, etc) are changed.

        // You can use this function to initialise any resources you might need,
        // but be careful - it will be called on the audio thread, not the GUI thread.

        // For more details, see the help for AudioProcessor::prepareToPlay()
    }

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        // Your audio-processing code goes here!

        // For more details, see the help for AudioProcessor::getNextAudioBlock()

        // Right now we are not producing any data, in which case we need to clear the buffer
        // (to prevent the output of random noise)
        bufferToFill.clearActiveBufferRegion();
    }

    void releaseResources() override
    {
        // This will be called when the audio device stops, or when it is being
        // restarted due to a setting change.

        // For more details, see the help for AudioProcessor::releaseResources()
    }

    //=======================================================================
    void paint (Graphics& g) override
    {
        // (Our component is opaque, so we must completely fill the background with a solid colour)
//        g.fillAll (Colours::black);


        // You can add your drawing code here!
    }

    void resized() override
    {
        // This is called when the MainContentComponent is resized.
        // If you add any child components, this is where you should
        // update their positions.
        mainTab.setBounds(getLocalBounds());
    }


private:
    //==============================================================================

    // Your private member variables go here...
    MainTab mainTab;
    LookAndFeel_V1 lookAndFeel;
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()     { return new MainContentComponent(); }


#endif  // MAINCOMPONENT_H_INCLUDED
