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
#include <essentia/essentiamath.h>
#include <essentia/pool.h>

#include "Tools.h"

namespace Muce {
    using namespace std;
    using namespace essentia;
    using namespace essentia::standard;
    
    //Feature map typedefs
    typedef std::map<std::string, std::vector<Real> > RealMap;
    typedef std::map<std::string, std::vector< std::vector<Real> > > VectorMap;
    typedef std::map<std::string, std::vector<Real> >::iterator RealMapIter;
    typedef std::map<std::string, std::vector< std::vector<Real> > >::iterator VectorMapIter;
    

    class Extraction : public ThreadWithProgressWindow {
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
        File threadAudioFolder;
        bool threadWriteOnsets;
        Pool threadFolderPool;
        
        Pool extractFeaturesFromFolder(const File& audioFolder, bool writeOnsets);
        Pool extractFeaturesFromOnsets(vector<vector<Real> >& slices, Real BPM);
        Pool extractFeatures(const vector<Real>& audio, Real BPM);
        
        
        //Music Hack Day
        void writeLoop(float onsetTime, const vector<Real>& audio, float BPM, String outFileName);
        vector<Real> randomLoop(const vector<Real>& onsetTimes, const vector<Real>& audio, Real BPM, String outFilename);
        vector<Real> firstLoop(const vector<Real>& onsetTimes, const vector<Real>& audio, Real BPM, String outFilename);
        
        StringArray featuresInPool(const Pool& pool);
        
        Tools tools;
        
        bool threaded = true;
        void run() override;
        
    private:
        //Spectral
        ScopedPointer<Algorithm> frameCutter, window, spec, mfcc;
        
        ScopedPointer<Algorithm> centroid;
        
        ScopedPointer<Algorithm> spectralFlatness;

        //Central Moments
        ScopedPointer<Algorithm> centralMoments, distShape;
        
        ScopedPointer<Algorithm> bands;
        
        //Global
        ScopedPointer<Algorithm> RMS, zcr, lat, envelope, tct;
        
        ScopedPointer<Algorithm> yamlOutput, poolAggregator;
        
        ScopedPointer<Algorithm> pitch;
        
        // FrameCutter -> Windowing -> Spectrum
        std::vector<Real> frame, windowedFrame;
        
        // Spectrum -> MFCC
        std::vector<Real> spectrum, mfccCoeffs, mfccBands;
        
        // Bands
        vector<Real> bandsVector;
        
        //Spectral Centroid
        Real spectralCentroid;
        
        //MHD descriptors
        Real pitchReal, pitchConfidence;
        
        Real spectralFlatnessReal;
        
        vector<Real> moments;
        
        Real spread, skewness, kurtosis;
        
        Real zcrReal;
        
        Real latReal;
        
        vector<Real> envelopeSignal;
        
        Real tctReal;        
    };
}

#endif  // EXTRACTION_H_INCLUDED
