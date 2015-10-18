/*
  ==============================================================================

    Audio.h
    Created: 18 Oct 2015 11:58:36am
    Author:  Cárthach Ó Nuanáin

  ==============================================================================
*/

#ifndef AUDIO_H_INCLUDED
#define AUDIO_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

#include <essentia/algorithmfactory.h>

namespace Muce {
    class Audio{
    public:
        AudioFormatManager formatManager;
        Audio()
        {
            formatManager.registerBasicFormats();
        }
        
        ~Audio() {};
        
        static std::vector<float> hannWindow(int size);
        static Array<File> getAudioFiles(const File& audioFolder);
        static bool vectorToAudioFile(const std::vector<essentia::Real> signal, const String fileName);
        
        AudioSampleBuffer audioFileToSampleBuffer(const File audioFile);
        std::vector<essentia::Real> audioFileToVector(const File audioFile);        
    };
}

#endif  // AUDIO_H_INCLUDED
