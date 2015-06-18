/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-12 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/
#ifndef AUDIOPLAYBACK_INCLUDED
#define AUDIOPLAYBACK_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "MyIncludes.h"

//==============================================================================
class DemoThumbnailComp  : public Component,
                           public ChangeListener,
                           public FileDragAndDropTarget,
                           public ChangeBroadcaster,
                           private ScrollBar::Listener,
                           private Timer
{
public:
    bool showOnsetSimilarities = false;
    
    DemoThumbnailComp (AudioFormatManager& formatManager, AudioTransportSource& transportSource_,Slider& slider)
        : transportSource (transportSource_),
          zoomSlider (slider),
          scrollbar (false),
          thumbnailCache (5),
          thumbnail (512, formatManager, thumbnailCache),
          isFollowingTransport (false)
    {
        thumbnail.addChangeListener (this);

        addAndMakeVisible (scrollbar);
        scrollbar.setRangeLimits (visibleRange);
        scrollbar.setAutoHide (false);
        scrollbar.addListener (this);

        currentPositionMarker.setFill (Colours::white.withAlpha (0.85f));
        addAndMakeVisible (currentPositionMarker);
    }

    ~DemoThumbnailComp()
    {
        scrollbar.removeListener (this);
        thumbnail.removeChangeListener (this);
    }

    void setFile (const File& file)
    {
        if (! file.isDirectory())
        {
            thumbnail.setSource (new FileInputSource (file));
            const Range<double> newRange (0.0, thumbnail.getTotalLength());
            scrollbar.setRangeLimits (newRange);
            setRange (newRange);

            startTimerHz (40);
        }
    }

    File getLastDroppedFile() const noexcept                    { return lastFileDropped; }

    void setZoomFactor (double amount)
    {
        if (thumbnail.getTotalLength() > 0)
        {
            const double newScale = jmax (0.001, thumbnail.getTotalLength() * (1.0 - jlimit (0.0, 0.99, amount)));
            const double timeAtCentre = xToTime (getWidth() / 2.0f);
            setRange (Range<double> (timeAtCentre - newScale * 0.5, timeAtCentre + newScale * 0.5));
        }
    }

    void setRange (Range<double> newRange)
    {
        visibleRange = newRange;
        scrollbar.setCurrentRange (visibleRange);
        updateCursorPosition();
        updateOnsetMarkers();
        if(showOnsetSimilarities)
            updateOnsetSliders();
        repaint();
    }

    void setFollowsTransport (bool shouldFollow)
    {
        isFollowingTransport = shouldFollow;
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black);
        g.setColour (Colours::lime);

        if (thumbnail.getTotalLength() > 0.0)
        {
            Rectangle<int> thumbArea (getLocalBounds());
            thumbArea.removeFromBottom (scrollbar.getHeight() + 4);
            thumbnail.drawChannels (g, thumbArea.reduced (2),
                                    visibleRange.getStart(), visibleRange.getEnd(), 1.0f);
        }
        else
        {
            g.setFont (14.0f);
            g.drawFittedText ("(No audio file selected)", getLocalBounds(), Justification::centred, 2);
        }
    }

    void resized() override
    {
        scrollbar.setBounds (getLocalBounds().removeFromBottom (14).reduced (2));
    }

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        // this method is called by the thumbnail when it has changed, so we should repaint it..
        repaint();
    }

    bool isInterestedInFileDrag (const StringArray& /*files*/) override
    {
        return true;
    }

    void filesDropped (const StringArray& files, int /*x*/, int /*y*/) override
    {
        lastFileDropped = File (files[0]);
        sendChangeMessage();
    }

    void mouseDown (const MouseEvent& e) override
    {
        mouseDrag (e);
    }

    void mouseDrag (const MouseEvent& e) override
    {
        if (canMoveTransport())
            transportSource.setPosition (jmax (0.0, xToTime ((float) e.x)));
    }

    void mouseUp (const MouseEvent&) override
    {
        transportSource.start();
    }

    void mouseWheelMove (const MouseEvent&, const MouseWheelDetails& wheel) override
    {
        if (thumbnail.getTotalLength() > 0.0)
        {
            double newStart = visibleRange.getStart() - wheel.deltaX * (visibleRange.getLength()) / 10.0;
            newStart = jlimit (0.0, jmax (0.0, thumbnail.getTotalLength() - (visibleRange.getLength())), newStart);

            if (canMoveTransport())
                setRange (Range<double> (newStart, newStart + visibleRange.getLength()));

            if (wheel.deltaY != 0.0f)
                zoomSlider.setValue (zoomSlider.getValue() - wheel.deltaY);

            repaint();
        }
    }
    
    void setOnsetMarkers(const std::vector<float>& onsetTimes)
    {
        this->onsetTimes = onsetTimes;
        
        updateOnsetMarkers();
    }
    
private:
    AudioTransportSource& transportSource;
    Slider& zoomSlider;
    ScrollBar scrollbar;

    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;
    Range<double> visibleRange;
    bool isFollowingTransport;
    File lastFileDropped;
    std::vector<float> onsetTimes;
    DrawableRectangle currentPositionMarker;
    
    OwnedArray<DrawableRectangle> onsetMarkers;
    OwnedArray<Slider> onsetSliders;

    float timeToX (const double time) const
    {
        return getWidth() * (float) ((time - visibleRange.getStart()) / (visibleRange.getLength()));
    }

    double xToTime (const float x) const
    {
        return (x / getWidth()) * (visibleRange.getLength()) + visibleRange.getStart();
    }

    bool canMoveTransport() const noexcept
    {
        return ! (isFollowingTransport && transportSource.isPlaying());
    }

    void scrollBarMoved (ScrollBar* scrollBarThatHasMoved, double newRangeStart) override
    {
        if (scrollBarThatHasMoved == &scrollbar)
            if (! (isFollowingTransport && transportSource.isPlaying()))
                setRange (visibleRange.movedToStartAt (newRangeStart));
    }

    void timerCallback() override
    {
        if (canMoveTransport())
            updateCursorPosition();
        else
            setRange (visibleRange.movedToStartAt (transportSource.getCurrentPosition() - (visibleRange.getLength() / 2.0)));
    }

    void updateCursorPosition()
    {
        currentPositionMarker.setVisible (transportSource.isPlaying() || isMouseButtonDown());

        currentPositionMarker.setRectangle (Rectangle<float> (timeToX (transportSource.getCurrentPosition()) - 0.75f, 0,
                                                              1.5f, (float) (getHeight() - scrollbar.getHeight())));
    }
    
    void updateOnsetMarkers()
    {
        onsetMarkers.clearQuick(true);
           
        for(int i=0; i<onsetTimes.size(); i++) {
            onsetMarkers.add(new DrawableRectangle());
            onsetMarkers[i]->setRectangle (Rectangle<float> (timeToX(onsetTimes[i]) - 0.75f, 0,
                                                             1.5f, (float) (getHeight() - scrollbar.getHeight())));
            
            addAndMakeVisible(onsetMarkers[i]);
            
            onsetMarkers[i]->setFill (Colours::white.withAlpha (0.85f));
        }
    }
    
    void updateOnsetSliders()
    {
        onsetSliders.clearQuick(true);
        
        for(int i=0; i<onsetTimes.size(); i++) {
            onsetSliders.add(new Slider());
            
            addAndMakeVisible (onsetSliders[i]);
            onsetSliders[i]->setSliderStyle (Slider::LinearBarVertical);
            
            onsetSliders[i]->setTopLeftPosition(timeToX(onsetTimes[i]) - 0.75f, 0);
            float width =  timeToX(onsetTimes[i+1]) - timeToX(onsetTimes[i]);
            onsetSliders[i]->setSize(width , (getHeight() - scrollbar.getHeight()));
            
            onsetSliders[i]->setRange(0.0,1.0);
            onsetSliders[i]->setValue(1.0);
            onsetSliders[i]->setColour(Slider::ColourIds::thumbColourId, Colours::blue.withAlpha((float)0.4));
            onsetSliders[i]->setColour(Slider::ColourIds::textBoxTextColourId, Colours::white);
        }
    }
};

//==============================================================================
class AudioPlaybackDemo  : public Component,
                           private FileBrowserListener,
                           public Button::Listener,
                           public Slider::Listener,
                           private ChangeListener
{
public:
    Rectangle<int> controlsRight;
    Rectangle<int> controlsLeft;
    ScopedPointer<DemoThumbnailComp> thumbnail;
    AudioFormatManager formatManager;
    EssentiaExtractor extractor;
    AudioTransportSource transportSource;
    ScopedPointer<AudioFormatReaderSource> currentAudioFileSource;

    
    
    AudioPlaybackDemo(AudioDeviceManager* deviceManager) :
          extractor(&formatManager),
          thread ("audio file preview"),
          directoryList (nullptr, thread),
          fileTreeComp (directoryList)
    {
        addAndMakeVisible (zoomLabel);
        zoomLabel.setText ("zoom:", dontSendNotification);
        zoomLabel.setFont (Font (15.00f, Font::plain));
        zoomLabel.setJustificationType (Justification::centredRight);
        zoomLabel.setEditable (false, false, false);
        zoomLabel.setColour (TextEditor::textColourId, Colours::black);
        zoomLabel.setColour (TextEditor::backgroundColourId, Colour (0x00000000));
        
        addAndMakeVisible (volumeLabel);
        volumeLabel.setText ("Gain:", dontSendNotification);
        volumeLabel.setFont (Font (15.00f, Font::plain));
        volumeLabel.setJustificationType (Justification::centredRight);
        volumeLabel.setEditable (false, false, false);
        volumeLabel.setColour (TextEditor::textColourId, Colours::black);
        volumeLabel.setColour (TextEditor::backgroundColourId, Colour (0x00000000));

        addAndMakeVisible (followTransportButton);
        followTransportButton.setButtonText ("Follow Transport");
        followTransportButton.addListener (this);
        
        addAndMakeVisible (loopButton);
        loopButton.setButtonText ("Loop");
        loopButton.addListener (this);

//        addAndMakeVisible (explanation);
//        explanation.setText ("Select an audio file in the treeview above, and this page will display its waveform, and let you play it..", dontSendNotification);
//        explanation.setFont (Font (14.00f, Font::plain));
//        explanation.setJustificationType (Justification::bottomRight);
//        explanation.setEditable (false, false, false);
//        explanation.setColour (TextEditor::textColourId, Colours::black);
//        explanation.setColour (TextEditor::backgroundColourId, Colour (0x00000000));

        addAndMakeVisible (zoomSlider);
        zoomSlider.setRange (0, 1, 0);
        zoomSlider.setSliderStyle (Slider::LinearHorizontal);
        zoomSlider.setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
        zoomSlider.addListener (this);
        zoomSlider.setSkewFactor (2);
        
        addAndMakeVisible (volumeSlider);
        volumeSlider.setRange (0, 1, 0);
        volumeSlider.setSliderStyle (Slider::LinearHorizontal);
        volumeSlider.setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
        volumeSlider.addListener (this);
        volumeSlider.setSkewFactor (2);
        volumeSlider.setValue(1.0);

        addAndMakeVisible (thumbnail = new DemoThumbnailComp (formatManager, transportSource, zoomSlider));
        thumbnail->addChangeListener (this);

        addAndMakeVisible (startStopButton);
        startStopButton.setButtonText ("Play/Stop");
        startStopButton.addListener (this);
        startStopButton.setColour (TextButton::buttonColourId, Colour (0xff79ed7f));
                
//        addAndMakeVisible (extractFeaturesButton);
//        extractFeaturesButton.setButtonText ("Extract Features");
//        extractFeaturesButton.addListener (this);
//        extractFeaturesButton.setColour (TextButton::buttonColourId, Colour (0xff79ed7f));
        

//        addAndMakeVisible (fileTreeComp);

        // audio setup
        formatManager.registerBasicFormats();

        directoryList.setDirectory (File::getSpecialLocation (File::userHomeDirectory), true, true);
        thread.startThread (3);

//        fileTreeComp.setColour (FileTreeComponent::backgroundColourId, Colours::lightgrey.withAlpha (0.6f));
//        fileTreeComp.addListener (this);
        
        this->deviceManager = deviceManager;
        deviceManager->addAudioCallback (&audioSourcePlayer);
        audioSourcePlayer.setSource (&transportSource);
        
        setOpaque (true);
    }

    ~AudioPlaybackDemo()
    {
        transportSource.setSource (nullptr);
        audioSourcePlayer.setSource (nullptr);

        deviceManager->removeAudioCallback (&audioSourcePlayer);
//        fileTreeComp.removeListener (this);
        thumbnail->removeChangeListener (this);
        followTransportButton.removeListener (this);
        zoomSlider.removeListener (this);
    }

    void paint (Graphics& g) override
    {
        g.setColour (Colour::greyLevel (0.2f));
        g.fillAll();
    }

    void resized() override
    {
        //Get Component size
        Rectangle<int> r (getLocalBounds().reduced (4).withY(50));
        thumbnail->setBounds (r.removeFromTop (200));
        controlsLeft = Rectangle<int>(r);
        
        controlsRight = controlsLeft.removeFromRight(controlsLeft.getWidth()/2);
//        explanation.setBounds (controls.removeFromRight (controls.getWidth() / 3));
        Rectangle<int> zoom (controlsLeft.removeFromTop (25));
        controlsRight.removeFromTop(25);
        zoomLabel.setBounds (zoom.removeFromLeft (50));
        zoomSlider.setBounds (zoom);
        
        Rectangle<int> buttonPos = controlsLeft.withSize(100, 50);
        startStopButton.setBounds (buttonPos);
        followTransportButton.setTopLeftPosition((buttonPos.getX()+100), buttonPos.getY());
        followTransportButton.setSize(150,50);
        loopButton.setTopLeftPosition(followTransportButton.getX()+130, followTransportButton.getY());
        loopButton.setSize(100,50);
        
        Point<int> similarityPos = Point<int>(4, controlsRight.getY()+25);
        
        volumeLabel.setTopLeftPosition(similarityPos.getX(), similarityPos.getY()+31);
        volumeLabel.setSize(75 , 25);
        
        volumeSlider.setTopLeftPosition(similarityPos.getX()+volumeLabel.getWidth(), similarityPos.getY()+30);
        volumeSlider.setSize(400, 25);

//        fileTreeComp.setBounds (r);
    }
    
    //==============================================================================
    virtual void showFile (const File& file)
    {
        loadFileIntoTransport (file);
        
        zoomSlider.setValue (0, dontSendNotification);
        thumbnail->setFile (file);
    }
    
    virtual void buttonClicked (Button* buttonThatWasClicked)
    {
        if (buttonThatWasClicked == &startStopButton)
        {
            if (transportSource.isPlaying())
            {
                transportSource.stop();
            }
            else
            {
                transportSource.setPosition (0);
                transportSource.start();
                transportSource.setLooping(true);
            }
        }
        else if (buttonThatWasClicked == &followTransportButton)
        {
            thumbnail->setFollowsTransport (followTransportButton.getToggleState());
        }
        else if (buttonThatWasClicked == &loopButton)
        {
            currentAudioFileSource->setLooping(loopButton.getToggleState());
        }
    }
    
protected:
    TextButton startStopButton, extractFeaturesButton;

    
    void sliderValueChanged (Slider* sliderThatWasMoved) override
    {
        if (sliderThatWasMoved == &zoomSlider)
            thumbnail->setZoomFactor (zoomSlider.getValue());
        if (sliderThatWasMoved == &volumeSlider)
            transportSource.setGain(volumeSlider.getValue());
    }

private:
    AudioDeviceManager* deviceManager;
    TimeSliceThread thread;
    DirectoryContentsList directoryList;

    AudioSourcePlayer audioSourcePlayer;

    Label zoomLabel, explanation, volumeLabel;
    Slider zoomSlider, volumeSlider;
    ToggleButton followTransportButton, loopButton;
    
    FileTreeComponent fileTreeComp;
    
    
    void loadFileIntoTransport (const File& audioFile)
    {
        // unload the previous file source and delete it..
        transportSource.stop();
        transportSource.setSource (nullptr);
        currentAudioFileSource = nullptr;

        AudioFormatReader* reader = formatManager.createReaderFor (audioFile);

        if (reader != nullptr)
        {
            currentAudioFileSource = new AudioFormatReaderSource (reader, true);

            // ..and plug it into our transport source
            transportSource.setSource (currentAudioFileSource,
                                       32768,                   // tells it to buffer this many samples ahead
                                       &thread,                 // this is the background thread to use for reading-ahead
                                       reader->sampleRate);     // allows for sample rate correctionset
        }
        

    }

    void selectionChanged() override
    {
        showFile (fileTreeComp.getSelectedFile());
    }

    void fileClicked (const File&, const MouseEvent&) override          {}
    void fileDoubleClicked (const File&) override                       {}
    void browserRootChanged (const File&) override                      {}



    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (source == thumbnail)
            showFile (thumbnail->getLastDroppedFile());
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPlaybackDemo);
};

#endif
