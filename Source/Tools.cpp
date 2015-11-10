/*
  ==============================================================================

    Audio.cpp
    Created: 18 Oct 2015 11:58:36am
    Author:  Cárthach Ó Nuanáin

  ==============================================================================
*/

#include "Tools.h"

namespace Muce {
    AudioSampleBuffer Tools::audioFileToSampleBuffer(const File audioFile)
    {
        //Read audio into buffer
        ScopedPointer<AudioFormatReader> reader;
        
        reader = formatManager.createReaderFor(audioFile);
        
        AudioSampleBuffer buffer(reader->numChannels, reader->lengthInSamples);
        reader->read(&buffer, 0, reader->lengthInSamples, 0, true, true);
        
        return buffer;
    }
    
    std::vector<essentia::Real> Tools::audioFileToVector(const File audioFile)
    {
        AudioSampleBuffer buffer = audioFileToSampleBuffer(audioFile);
        
        //Convert buffer to Essentia vector and mono-ise as necessary
        std::vector<essentia::Real> essentiaSignal(buffer.getNumSamples());
        
        const float* leftChannelPtr = buffer.getReadPointer(0);
        const float* rightChannelPtr = NULL;
        
        if (buffer.getNumChannels() == 2)
            rightChannelPtr = buffer.getReadPointer(1);
        
        for(int i=0; i<buffer.getNumSamples(); i++) {
            essentiaSignal[i] = leftChannelPtr[i];
            if (buffer.getNumChannels() == 2) {
                essentiaSignal[i] = (leftChannelPtr[i] + rightChannelPtr[i]) / 2.0;
            }
        }
        
        return essentiaSignal;
    }
    
    std::vector<float> Tools::hannWindow(int size){
        std::vector<float> window;
        for (int i = 0; i < size; i++) {
            double multiplier = 0.5 * (1.0 - cos(2.0*M_PI*(float)i/(float)size));
            window.push_back(multiplier);
        }
        return window;
    }
    
    Array<File> Tools::getAudioFiles(const File& audioFolder)
    {
        Array<File> audioFiles;
        
        DirectoryIterator iter (audioFolder, false, "*.mp3;*.wav");
        
        while (iter.next())
        {
            File theFileItFound (iter.getFile());
            audioFiles.add(theFileItFound);
        }
        
        return audioFiles;
    }
    
    bool Tools::vectorToAudioFile(const std::vector<essentia::Real> signal, const String fileName)
    {
        ScopedPointer<WavAudioFormat> wavFormat =  new WavAudioFormat();
        
        File output(fileName);
        FileOutputStream *fost = output.createOutputStream();
        
        AudioFormatWriter* writer = wavFormat->createWriterFor(fost, 44100, 1, 16, StringPairArray(), 0);
        
        const float* signalPtr[1];
        signalPtr[0] = &signal[0];
        
        writer->writeFromFloatArrays(signalPtr, 1, signal.size());
        delete writer;
        
        return true;
    }
    
    std::vector<essentia::Real> Tools::removeLabels(essentia::Pool& pool)
    {
        //Train Classifier
        if (pool.contains<std::vector<essentia::Real> >("labels")) {
            std::vector<essentia::Real> labelsVector = pool.value<std::vector<essentia::Real> >("labels");
            pool.remove("labels");
            
            return labelsVector;
        }
    }
    
    cv::Mat Tools::poolToMat(const essentia::Pool& pool)
    {
        using namespace cv;
        
        std::map<std::string, std::vector<essentia::Real> > realFeatures = pool.getRealPool();
        std::map<std::string, std::vector< std::vector<essentia::Real> > > vectorFeatures = pool.getVectorRealPool();
        
        cv::Mat mat;
        
        //No features return empty mat
        if(realFeatures.empty() && vectorFeatures.empty())
            return mat;
        
        //Do real Features first
        int noOfFeatures = realFeatures.size();
        RealMapIter realIterator = realFeatures.begin();
        int noOfInstances = realIterator->second.size();
        
        Mat realFeaturesMatrix(noOfInstances, noOfFeatures, DataType<float>::type);
        
        int i=0;
        for(; realIterator != realFeatures.end(); realIterator++) {
            for(int j=0; j<realIterator->second.size(); j++)
                realFeaturesMatrix.at<float>(j,i) = realIterator->second[j];
            i++;
        }
        
        //If there are no vectorFeatures we are done
        if(vectorFeatures.empty())
            return realFeaturesMatrix;
        
        //Vector Features
        
        VectorMapIter vectorIterator;
        int noOfVectorFeatures = 0;
        
        //noOfVectorFeatures = the sum of the dimensions of each feature
        for(vectorIterator = vectorFeatures.begin(); vectorIterator != vectorFeatures.end(); vectorIterator++) {
            if(!vectorIterator->second.empty()) {
                noOfVectorFeatures += vectorIterator->second[0].size(); //Get the dimensionality from the first instance
                jassert(noOfInstances == vectorIterator->second.size()); //The number of instances should agree with the Reals
            }
        }
        
        Mat vectorFeaturesMatrix(noOfInstances, noOfVectorFeatures, DataType<float>::type);
        
        //Instances
        for(int i=0; i<noOfInstances;i++) {
            //Vector
            int rowCounter = 0;
            for(vectorIterator = vectorFeatures.begin(); vectorIterator != vectorFeatures.end(); vectorIterator++) {
                for(int j=0; j<vectorIterator->second[i].size(); j++) {
                    vectorFeaturesMatrix.at<float>(i,rowCounter) = vectorIterator->second[i][j];
                    rowCounter++;
                }
                
            }
        }
        
        //Concatenate
        hconcat(realFeaturesMatrix, vectorFeaturesMatrix, mat);
        
        return mat;
    }

    
    //vector<Real> EssentiaExtractor::extractOnsetTimes(const vector<Real>& audio)
    //{
    //    Algorithm* extractoronsetrate = AlgorithmFactory::create("OnsetRate");
    //
    //    Real onsetRate;
    //    vector<Real> onsets;
    //
    //    extractoronsetrate->input("signal").set(audio);
    //    extractoronsetrate->output("onsets").set(onsets);
    //    extractoronsetrate->output("onsetRate").set(onsetRate);
    //
    //    extractoronsetrate->compute();
    //
    //    delete extractoronsetrate;
    //
    //    return onsets;
    //}

    
}