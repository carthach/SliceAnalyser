#ifndef GENAUDIO_INCLUDED
#define GENAUDIO_INCLUDED

#include "AudioPlaybackDemo.h"
#include "EssentiaExtractor.h"
#include "MyIncludes.h"
#include "TargetAudioComponent.h"
#include "DataComponent.h"

class GenAudioComponent : public AudioPlaybackDemo
{

public:
    GenAudioComponent(AudioDeviceManager* deviceManager, TargetAudioComponent* targetAudioComponent, DataComponent* dataComponent) : AudioPlaybackDemo(deviceManager)
    {
        
        addAndMakeVisible (fileLoadButton);
        fileLoadButton.setButtonText ("Generate Sample");
        fileLoadButton.addListener(this);
        fileLoadButton.setColour (TextButton::buttonColourId, Colour (0xff79ed7f));
        
        addAndMakeVisible (linkTransportButton);
        linkTransportButton.setButtonText("Link Transports");
        linkTransportButton.addListener(this);
        linkTransportButton.setColour (TextButton::buttonColourId, Colour (0xff79ed7f));
        
        addAndMakeVisible (similaritySlider);
        similaritySlider.setRange (0, 1, 0);
        similaritySlider.setSliderStyle (Slider::LinearHorizontal);
        similaritySlider.setSkewFactor(5.0);
        similaritySlider.setTextBoxStyle (Slider::TextBoxRight, false, 80, 20);
        similaritySlider.addListener (this);
//        similaritySlider.setSkewFactor (2);
        
        addAndMakeVisible (similarityLabel);
        similarityLabel.setText ("Similarity:", dontSendNotification);
        similarityLabel.setFont (Font (15.00f, Font::plain));
        similarityLabel.setJustificationType (Justification::centredRight);
        similarityLabel.setColour (TextEditor::textColourId, Colours::black);
        similarityLabel.setColour (TextEditor::backgroundColourId, Colour (0x00000000));
        
        addAndMakeVisible (smoothStartSlider);
        smoothStartSlider.setRange (0, 1, 0);
        smoothStartSlider.setSliderStyle (Slider::LinearHorizontal);
        smoothStartSlider.setTextBoxStyle (Slider::TextBoxRight, false, 50, 20);
        smoothStartSlider.addListener (this);
        
        addAndMakeVisible (smoothStartLabel);
        smoothStartLabel.setText ("Smooth Start:", dontSendNotification);
        smoothStartLabel.setFont (Font (15.00f, Font::plain));
        smoothStartLabel.setJustificationType (Justification::centredRight);
        smoothStartLabel.setColour (TextEditor::textColourId, Colours::black);
        smoothStartLabel.setColour (TextEditor::backgroundColourId, Colour (0x00000000));
        
        addAndMakeVisible (smoothEndSlider);
        smoothEndSlider.setRange (0, 1, 0);
        smoothEndSlider.setSliderStyle (Slider::LinearHorizontal);
        smoothEndSlider.setTextBoxStyle (Slider::TextBoxRight, false, 50, 20);
        smoothEndSlider.addListener (this);
        smoothEndSlider.setValue(1.0);
        
        addAndMakeVisible (smoothEndLabel);
        smoothEndLabel.setText ("Smooth Start:", dontSendNotification);
        smoothEndLabel.setFont (Font (15.00f, Font::plain));
        smoothEndLabel.setJustificationType (Justification::centredRight);
        smoothEndLabel.setEditable (false, false, false);
        smoothEndLabel.setColour (TextEditor::textColourId, Colours::black);
        smoothEndLabel.setColour (TextEditor::backgroundColourId, Colour (0x00000000));
        
        this->targetAudioComponent = targetAudioComponent;
        this->dataComponent = dataComponent;
    }
    
private:
    TextButton fileLoadButton;
    ToggleButton linkTransportButton;
    TargetAudioComponent* targetAudioComponent;
    DataComponent* dataComponent;
    cv::Mat distanceMatrix;
    cv::Mat similarityMatrix;
    bool transportLinked = false;
    Slider similaritySlider, smoothStartSlider, smoothEndSlider;
    Label similarityLabel, smoothStartLabel, smoothEndLabel;

    bool distanceMatrixGenerated = false;
    
    void showFile (const File& file) override
    {
        AudioPlaybackDemo::showFile(file);
        
        std::vector<float> signal = extractor.audioFileToVector(file);
        std::vector<float> onsetTimes = extractor.extractOnsetTimes(signal);
        
        thumbnail->setOnsetMarkers(onsetTimes);
    }
        
    void resized () override
    {
        AudioPlaybackDemo::resized();
        
        controlsRight.getTopRight();
        Rectangle<int> buttonPos = controlsRight.withSize(125, 50);
        
        buttonPos.setPosition(controlsRight.getTopRight().getX()-buttonPos.getWidth(), controlsRight.getTopRight().getY());
        fileLoadButton.setBounds(buttonPos);
        buttonPos.setPosition(buttonPos.getX()-fileLoadButton.getWidth(), controlsRight.getTopRight().getY());
        linkTransportButton.setBounds(buttonPos);
        buttonPos.setPosition(buttonPos.getX()-(fileLoadButton.getWidth()+linkTransportButton.getWidth()), controlsRight.getTopRight().getY());
        
        Point<int> similarityPos = Point<int>(4, controlsRight.getY()+50);
        
        similarityLabel.setTopLeftPosition(similarityPos.getX(), similarityPos.getY()+31);
        similarityLabel.setSize(75 , 25);
        
        similaritySlider.setTopLeftPosition(similarityPos.getX()+similarityLabel.getWidth(), similarityPos.getY()+30);
        similaritySlider.setSize(400, 25);
        
        smoothStartLabel.setTopLeftPosition(similarityPos.getX(), similarityPos.getY()+61);
        smoothStartLabel.setSize(85 , 25);
        
        smoothStartSlider.setTopLeftPosition(similarityPos.getX()+similarityLabel.getWidth(), similarityPos.getY()+60);
        smoothStartSlider.setSize(400, 25);
        
        smoothEndLabel.setTopLeftPosition(similarityPos.getX(), similarityPos.getY()+91);
        smoothEndLabel.setSize(85, 25);
        
        smoothEndSlider.setTopLeftPosition(similarityPos.getX()+similarityLabel.getWidth(), similarityPos.getY()+90);
        smoothEndSlider.setSize(400, 25);
    }
    
    void buttonClicked (Button* buttonThatWasClicked) override
    {
        if(buttonThatWasClicked == &fileLoadButton) {
            generateDistanceMatrix(); //Need to figure out how not to recompute this everytime
            generateSimilarityMatrix();
            generateNewClip();
        } else if(buttonThatWasClicked == &linkTransportButton) {
            transportLinked = linkTransportButton.getToggleState();
        } else if (buttonThatWasClicked == &startStopButton && linkTransportButton.getToggleState())
        {
            if (targetAudioComponent->transportSource.isPlaying())
            {
                targetAudioComponent->transportSource.stop();
            }
            else
            {
                targetAudioComponent->transportSource.setPosition (0);
                targetAudioComponent->transportSource.start();
            }
        }
        AudioPlaybackDemo::buttonClicked(buttonThatWasClicked);
    }
    
    void normaliseFeatures(cv::Mat mat)
    {
        for(int i=0; i <mat.cols; i++) {
            cv::normalize(mat.col(i), mat.col(i), 0, 1, cv::NORM_MINMAX, CV_32F);
        }
    }
    
    float weightedDistance(const Array<float>& weights, cv::Mat vectorOne, cv::Mat vectorTwo)
    {
        float dist = 0.0f;
        
        //Make sure everything is okay
        jassert(weights.size() == vectorOne.cols);
        
        for(int i=0; i<weights.size(); i++) {
            float xminusy = vectorOne.at<float>(0,i) - vectorTwo.at<float>(0,i);
            xminusy = xminusy * xminusy;
            dist += weights[i] * xminusy;
        }
        
        dist = sqrt(dist);
        
        return dist;
    }
    
    void generateDistanceMatrix()
    {
        cv::Mat targetMatrix = targetAudioComponent->featureMatrix;
        cv::Mat datasetMatrix = dataComponent->featureMatrix;

        StringArray headerData = dataComponent->getHeaderData();
        cv::Mat reducedTargetMatrix = dataComponent->getSelectedFeatureMatrix(headerData, targetMatrix);
        cv::Mat reducedDatasetMatrix = dataComponent->getSelectedFeatureMatrix(headerData, datasetMatrix);
        


        //Merge matrices
        reducedDatasetMatrix.push_back(reducedTargetMatrix);
        
        //Normalise
        normaliseFeatures(reducedDatasetMatrix);
        
        //Delace
        reducedTargetMatrix = reducedDatasetMatrix.rowRange(reducedDatasetMatrix.rows-reducedTargetMatrix.rows, reducedDatasetMatrix.rows);
        
        for(int i=0 ;i<reducedTargetMatrix.rows; i++)
            reducedDatasetMatrix.pop_back();
        
        distanceMatrix.create(reducedDatasetMatrix.rows, reducedTargetMatrix.rows, cv::DataType<float>::type);
        
        for(int i=0; i<reducedTargetMatrix.rows; i++) { //No of onsets
            for(int j=0; j<reducedDatasetMatrix.rows; j++) { //No of dataset slices
                
//                float dist = norm(reducedTargetMatrix.row(i), reducedDatasetMatrix.row(j));
                float dist = weightedDistance(dataComponent->tableComponent.weights, reducedTargetMatrix.row(i), reducedDatasetMatrix.row(j));
                
                distanceMatrix.at<float>(j,i) = dist;
            }
        }
        
        distanceMatrixGenerated = true;
    }
    
    void generateSimilarityMatrix()
    {
        cv::sortIdx(distanceMatrix, similarityMatrix, cv::SORT_EVERY_COLUMN + cv::SORT_ASCENDING);
    }
    
    void sliderValueChanged (Slider* sliderThatWasMoved) override
    {
        AudioPlaybackDemo::sliderValueChanged(sliderThatWasMoved);
    }
    
    
    //This will generate a new vector from the database
    void generateNewClip()
    {
        std::vector<int> newOnsetPattern;
        
        float similarityFactor = similaritySlider.getValue();
        
        int sequenceIndex = (1.0-similarityFactor) * (float)(similarityMatrix.rows-1);
        
//        Random random;
//        
//        for(int i=0;i<similarityMatrix.cols; i++) {
//            if(sequenceIndex > 0)
//                newOnsetPattern.push_back(random.nextInt(sequenceIndex));
//            else
//                newOnsetPattern.push_back(0);
//        }
        
        similarityMatrix.row(sequenceIndex).copyTo(newOnsetPattern);
        
        TargetAudioComponent::CurrentTargetData* targetData = &targetAudioComponent->currentTargetData;
        
        std::vector<float> sampleVector(targetData->signal.size(), 0.0f);
        
        String audioFilenameRoot = dataComponent->datasetFolder.getFullPathName() + "/dataset/";
        
        //Fill the start of the buffer with 0.0f until the first onset
        for(int i=0; i < (targetData->onsetTimes[0] * 44100); i++)
            sampleVector.push_back(0.0f);
        
        //Place the new onsets at the sample times of the original
        for(int i=0; i<newOnsetPattern.size(); i++) {
            String audioFilename = audioFilenameRoot + "slice_" + String(newOnsetPattern[i]) + ".wav";
            vector<float> onsetVector = extractor.audioFileToVector(File(audioFilename));
            
            vector<float> hannWindow = extractor.hannWindow(onsetVector.size());
            
            int startSmoothSample = smoothStartSlider.getValue() * (float)onsetVector.size();
            int endSmoothSample = smoothEndSlider.getValue() * (float)onsetVector.size();
            
            int samplePos = targetData->onsetTimes[i] * 44100;
            
            for(int j=0; j<onsetVector.size(); j++) {
//                onsetVector[j] *= targetData->onsetPeakValues[i];
                if(j < startSmoothSample || j > endSmoothSample)
                    onsetVector[j] *= hannWindow[j];

                sampleVector[j+samplePos] += onsetVector[j];
                
            }
        }
        
        String audioFilename = audioFilenameRoot + "new.wav";
        
        if(File(audioFilename).existsAsFile())
            File(audioFilename).deleteFile();
        
        extractor.vectorToAudioFile(sampleVector, audioFilename);
        
        showFile(File(audioFilename));
    }
};


#endif