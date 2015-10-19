/*
  ==============================================================================

    Extraction.h
    Created: 18 Oct 2015 11:58:03am
    Author:  Cárthach Ó Nuanáin

  ==============================================================================
*/

#ifndef EXTRACTION_H_INCLUDED
#define EXTRACTION_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

#include <essentia/algorithmfactory.h>
#include <essentia/streaming/algorithms/poolstorage.h>
#include <essentia/scheduler/network.h>
#include <essentia/streaming/algorithms/vectorinput.h>

#include "Audio.h"

namespace Muce {
    using namespace std;
    using namespace essentia;
    using namespace essentia::streaming;
    using namespace essentia::scheduler;
    
    //Feature map typedefs
    typedef std::map<std::string, std::vector<Real> > RealMap;
    typedef std::map<std::string, std::vector< std::vector<Real> > > VectorMap;
    typedef std::map<std::string, std::vector<Real> >::iterator RealMapIter;
    typedef std::map<std::string, std::vector< std::vector<Real> > >::iterator VectorMapIter;
    

    class Extraction {
    public:
        int sliceID;
        Random random;
        
        Extraction();
        ~Extraction();
        void initAlgorithms();
        
        int sampleRate = 44100;
        int frameSize = 2048;
        int hopSize = 1024;
        
        //Declare global algorithms
        
        //Handling onsets
        vector<Real> extractOnsetTimes(const vector<Real>& audio);
        vector<vector<Real> > extractOnsets(const vector<Real>& onsetTimes, const vector<Real>& audio);
        vector<vector<Real> > extractOnsets(const vector<Real>& onsetTimes, const vector<Real>& audio, int numOnsets);
        
        vector<Real> extractPeakValues(const vector<vector<Real> >& slices);
        void writeOnsets(const vector<vector<Real> >& slices, const String outputRoot);

        //Load Features
        Pool loadFeatures(const String& jsonFileName);
        

        vector<Real> extractRhythmFeatures(const vector<Real>& audio);

        //Extract features for a vector of onsets
        
        //Build a whole dataset and output a json/yaml file
        Pool extractFeaturesFromFiles(const File& audioFolder, bool writeOnsets);
        Pool extractFeaturesFromOnsets(vector<vector<Real> >& slices, Real BPM);
        Pool extractFeatures(vector<Real>& audio, Real BPM);
        
        
        //Music Hack Day
        void writeLoop(float onsetTime, const vector<Real>& audio, float BPM, String outFileName);
        vector<Real> randomLoop(const vector<Real>& onsetTimes, const vector<Real>& audio, Real BPM, String outFilename);
        vector<Real> firstLoop(const vector<Real>& onsetTimes, const vector<Real>& audio, Real BPM, String outFilename);
        
        StringArray featuresInPool(const Pool& pool);
        
        Audio audioTools;
        
    private:
        //==========================ESSENTIA=========================
        
        VectorInput<Real>* vectorInput, *onsetVectorInput;
        
        //Spectral
        Algorithm* frameCutter, *window, *spec, *mfcc;
        Algorithm* centroid;
        Algorithm* spectralFlatness;

        //Central Moments
        Algorithm* centralMoments, *distShape;
        Algorithm* bands;
        
        //Global
        Algorithm* RMS, *zcr, *lat, *envelope, *tct;
        standard::Algorithm* yamlOutput, *poolAggregator;
        Algorithm* pitch;
        
        Algorithm* onsetRate;
        
        //This holds the framewise pool between files
        Pool framePool, onsetPool;
        vector<Real> frameBuffer;
        
        Network* network, *onsetVectorNetwork;
    };
}

#endif  // EXTRACTION_H_INCLUDED
