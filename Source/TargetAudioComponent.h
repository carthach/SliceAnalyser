#ifndef TARGETAUDIOCOMPONENT_INCLUDED
#define TARGETAUDIOCOMPONENT_INCLUDED

#include "AudioPlaybackDemo.h"



class TargetAudioComponent : public AudioPlaybackDemo
{
public:
    struct CurrentTargetData {
        std::vector<Real> signal;
        std::vector<Real> onsetTimes;
        std::vector<std::vector<Real> > onsets;
        std::vector<Real> onsetPeakValues;
    };
    
    cv::Mat featureMatrix;
    CurrentTargetData currentTargetData;
    
    OwnedArray<Slider> onsetSliders;
    
    TargetAudioComponent(AudioDeviceManager* deviceManager) : AudioPlaybackDemo(deviceManager)
    {
        addAndMakeVisible (fileLoadButton);
        fileLoadButton.setButtonText ("Load Sample");
        fileLoadButton.addListener(this);
        fileLoadButton.setColour (TextButton::buttonColourId, Colour (0xff79ed7f));
        

        addAndMakeVisible (showOnsetSimilarityButton);
        showOnsetSimilarityButton.setButtonText("Show Onset Similarity");
        showOnsetSimilarityButton.addListener(this);
        showOnsetSimilarityButton.setColour (TextButton::buttonColourId, Colour (0xff79ed7f));
    }
    
private:
    TextButton fileLoadButton;
    ToggleButton showOnsetSimilarityButton;
    
    Slider barSlider;

    void showFile (const File& file) override
    {
        AudioPlaybackDemo::showFile(file);
        
        currentTargetData.signal = extractor.audioFileToVector(file);
        currentTargetData.onsetTimes = extractor.extractOnsetTimes(currentTargetData.signal);
        currentTargetData.onsets = extractor.extractOnsets(currentTargetData.onsetTimes, currentTargetData.signal);
        currentTargetData.onsetPeakValues = extractor.extractPeakValues(currentTargetData.onsets);
        
        Pool features = extractor.extractFeatures(currentTargetData.onsets, 0.0f);
        featureMatrix = extractor.poolToMat(features);
        cv::Mat labels = extractor.clusterData(featureMatrix);
        std::vector<int> labelsVector;
        
        for(int i=0; i<labels.rows; i++)
            labelsVector.push_back(labels.at<int>(i,0));

        thumbnail->setOnsetColours(labelsVector);
        thumbnail->setOnsetMarkers(currentTargetData.onsetTimes);
        
    }
    
    void resized () override
    {
        AudioPlaybackDemo::resized();
        
        controlsRight.getTopRight();
        Rectangle<int> buttonPos = controlsRight.withSize(100, 50);
        buttonPos.setPosition(controlsRight.getTopRight().getX()-buttonPos.getWidth(), controlsRight.getTopRight().getY());
        
        fileLoadButton.setBounds(buttonPos);
        buttonPos.setPosition(buttonPos.getX()-fileLoadButton.getWidth(), controlsRight.getTopRight().getY());
        showOnsetSimilarityButton.setBounds(buttonPos);
        
        barSlider.setTopLeftPosition(buttonPos.getX()-10, buttonPos.getY()+50);
        barSlider.setSize(50, 150);
    }
    
    void buttonClicked (Button* buttonThatWasClicked) override
    {
        AudioPlaybackDemo::buttonClicked(buttonThatWasClicked);
        
        if (buttonThatWasClicked == &fileLoadButton)
        {
            FileChooser fc ("Choose a file to open...",
                            File::getCurrentWorkingDirectory(),
                            "*",
                            true);
            
            if (fc.browseForFileToOpen())
            {
                File chosenFile = fc.getResult();
                
                if(chosenFile.exists())
                    showFile (chosenFile);
            }
        } else if(buttonThatWasClicked == &showOnsetSimilarityButton) {
            AudioPlaybackDemo::thumbnail->showOnsetSimilarities = true;
            AudioPlaybackDemo::thumbnail->repaint();
        }
    }
    
//        void updateOnsetSliders()
//        {
//            onsetSliders.clearQuick(true);
//    
//            for(int i=0; i<currentTargetData.onsetTimes.size(); i++) {
//                onsetSliders.add(new Slider());
//    
//                addAndMakeVisible (onsetSliders[i]);
//                onsetSliders[i]->setSliderStyle (Slider::LinearBarVertical);
//                onsetSliders[i]->setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
//    
////                onsetMarkers[i]->setRectangle (Rectangle<float> (timeToX(onsetTimes[i]) - 0.75f, 150,
////                                                                 50.0f, (float) (getHeight() - scrollbar.getHeight())));
//                
//            }
//        }
};

#endif