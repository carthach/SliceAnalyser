/*
  ==============================================================================

    Audio.cpp
    Created: 18 Oct 2015 11:58:36am
    Author:  Cárthach Ó Nuanáin

  ==============================================================================
*/

#include "Audio.h"

namespace Muce {
    AudioSampleBuffer Audio::audioFileToSampleBuffer(const File audioFile)
    {
        //Read audio into buffer
        ScopedPointer<AudioFormatReader> reader;
        
        reader = formatManager.createReaderFor(audioFile);
        
        AudioSampleBuffer buffer(reader->numChannels, reader->lengthInSamples);
        reader->read(&buffer, 0, reader->lengthInSamples, 0, true, true);
        
        return buffer;
    }
    
    std::vector<essentia::Real> Audio::audioFileToVector(const File audioFile)
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
    
    std::vector<float> Audio::hannWindow(int size){
        std::vector<float> window;
        for (int i = 0; i < size; i++) {
            double multiplier = 0.5 * (1.0 - cos(2.0*M_PI*(float)i/(float)size));
            window.push_back(multiplier);
        }
        return window;
    }
    
    Array<File> Audio::getAudioFiles(const File& audioFolder)
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
    
    bool Audio::vectorToAudioFile(const std::vector<essentia::Real> signal, const String fileName)
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