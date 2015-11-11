//
//  Batch.cpp
//  rhythmCAT
//
//  Created by Cárthach Ó Nuanáin on 11/11/2015.
//
//

#include <stdio.h>

#include "../JuceLibraryCode/JuceHeader.h"
#include "Extraction.h"
#include "Tools.h"

using namespace Muce;

class ThreadBatch : public ThreadWithProgressWindow {

    Extraction& extraction;
    Tools& tools;
    
    File directory;
    bool writeOnsets;
    Pool pool;
    
    ThreadBatch(Extraction& extractionIn) : ThreadWithProgressWindow ("Building Dataset...", true, true), extraction(extractionIn), tools(extraction.tools)
    {
        
    }
    
    void run()
    {
        using namespace essentia;
        using namespace essentia::standard;
        
        StringArray algorithms = {"MFCC", "Centroid", "Flatness"};
        extraction.setupUserAlgorithms(algorithms);
        
        Array<File> audioFiles = tools.getAudioFiles(directory);
        
        File datasetRoot(directory.getFullPathName() + "/dataset/");
        if(datasetRoot.exists())
            datasetRoot.deleteRecursively();
        datasetRoot.createDirectory();
        
        int fileCounter = 0;
        int onsetCounter = 0;
        
        for(fileCounter=0; fileCounter<audioFiles.size(); fileCounter++)
        {
            
            
            //Input audio
            vector<Real> audio =  tools.audioFileToVector(audioFiles[fileCounter]);
            
            //Get slices
            vector<Real> onsetTimes = extraction.extractOnsetTimes(audio);
            
            //Then get the audio
            vector<vector<Real> > audioOnsets = extraction.extractOnsets(onsetTimes, audio);
            
            for(onsetCounter=0; onsetCounter < audioOnsets.size();onsetCounter++)
            {
                //Extract Features
                Pool onsetPool = extraction.extractFeatures(audioOnsets[onsetCounter], 120.0);
                
                //Write Onset
                if(writeOnsets) {
                    String audioSliceFilename = datasetRoot.getFullPathName() + "/" + tools.getAppendedFilename(audioFiles[fileCounter], "_slice_" + String(onsetCounter));
                    File audioSliceFile = File(audioSliceFilename);
                    tools.vectorToAudioFile(audioOnsets[onsetCounter], audioSliceFilename);
                }
                
                pool.merge(onsetPool, "append");
            }
        }
    }
    
    Pool batchExtract(const File & directory, bool writeOnsets)
    {
        this->directory = directory;
        pool.clear();
        runThread();
        return pool;
    }
    
};