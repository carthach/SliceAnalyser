/*
  ==============================================================================

    Extraction.cpp
    Created: 18 Oct 2015 11:58:03am
    Author:  Cárthach Ó Nuanáin

  ==============================================================================
*/

#include "Extraction.h"

namespace Muce {
    Extraction::Extraction()
    {
        initAlgorithms();
    }
    
    Extraction::~Extraction()
    {
        delete network;
    }
    
    void Extraction::initAlgorithms()
    {
        AlgorithmFactory& factory = streaming::AlgorithmFactory::instance();
        
        vectorInput = new VectorInput<Real>;
        
        //Stastical things
        float halfSampleRate = (float)sampleRate / 2.0;
        
        onsetRate = AlgorithmFactory::create("OnsetRate");
        
        frameCutter    = factory.create("FrameCutter",
                                        "frameSize", frameSize,
                                        "hopSize", hopSize);
        
        window     = factory.create("Windowing", "type", "hann");
        
        spec  = factory.create("Spectrum");
        
        mfcc  = factory.create("MFCC");
        
        vectorInput->output("data") >>  frameCutter->input("signal");
        
        // FrameCutter -> Windowing -> Spectrum
        frameCutter->output("frame")       >>  window->input("frame");
        window->output("frame")        >>  spec->input("frame");
        
        // Spectrum -> MFCC -> Pool
        spec->output("spectrum")  >>  mfcc->input("spectrum");
        
        mfcc->output("bands")     >>  NOWHERE;                          // we don't want the mel bands
        mfcc->output("mfcc")      >>  PC(framePool, "lowlevel.mfcc"); // store only the mfcc coeffs
        
        onsetVectorInput = new VectorInput<Real>();
        onsetVectorInput->output("data") >> onsetRate->input("signal");
        onsetRate->output("onsetTimes") >> PC(onsetPool, "onsetTimes");
        onsetRate->output("onsetRate") >> NOWHERE;
        
//        centroid = factory.create("Centroid", "range", halfSampleRate);
//        centralMoments = factory.create("CentralMoments", "range", halfSampleRate);
//        distShape = factory.create("DistributionShape");
//        
//        spectralFlatness = factory.create("FlatnessDB");
//        
//        //Energy Bands
//        //    Algorithm* bands = factory.create("ERBBands");
//        bands = factory.create("ERBBands");
//        
//        //Global
//        RMS = factory.create("RMS");
//        zcr = factory.create("ZeroCrossingRate");
//        lat = factory.create("LogAttackTime");
//        envelope = factory.create("Envelope");
//        tct = factory.create("TCToTotal");
        
        //Yaml Output in JSON format
        //Aggregate Pool stats
        string defaultStats[] = {"mean", "var"};
        poolAggregator = standard::AlgorithmFactory::create("PoolAggregator", "defaultStats", arrayToVector<string>(defaultStats));
        
        yamlOutput  = standard::AlgorithmFactory::create("YamlOutput", "format", "yaml");
        
        network = new Network(vectorInput);
        onsetVectorNetwork = new Network(onsetVectorInput);
    }
    
    vector<Real> Extraction::extractPeakValues(const vector<vector<Real> >& slices)
    {
        vector<Real> peakValues(slices.size(), 0.0f);
        for(int i=0; i<slices.size(); i++) {
            for(int j=0; j<slices[i].size(); j++) {
                float absSample = fabs(slices[i][j]);
                if (absSample > peakValues[i]) {
                    peakValues[i] = absSample;
                }
            }
        }
        return peakValues;
    }
    
    vector<Real> Extraction::extractOnsetTimes(const vector<Real>& audio)
    {
        
        onsetVectorNetwork->reset();
        onsetVectorInput->setVector(&audio);
        onsetVectorNetwork->run();
        
        return onsetPool.value<vector<Real>>("onsetTimes");
    }
    
    
    
    vector<Real> Extraction::extractRhythmFeatures(const vector<Real>& audio)
    {
        ScopedPointer<Algorithm> rhythmExtractor = AlgorithmFactory::create("RhythmExtractor2013", "method", "degara");
        
        Pool pool;
        
        VectorInput<Real>* v;
        v = new VectorInput<Real>;
        v->declareParameters();
        
        v->setVector(&audio);
        v->output("data") >> rhythmExtractor->input("signal");
        rhythmExtractor->output("ticks") >> NOWHERE;
        rhythmExtractor->output("confidence") >> NOWHERE;
        rhythmExtractor->output("bpm") >> PC(pool, "bpm");
        rhythmExtractor->output("estimates") >> NOWHERE;
        rhythmExtractor->output("bpmIntervals") >> NOWHERE;
        
        Network n(v);
        n.run();
        
        std::cout << "rhythmFeatures" << "\n";
        
        vector<Real> features;
        features.push_back(pool.value<Real>("bpm"));
        
        n.clear();
        
        return features;
    }
    
    //vector<Real> Extraction::getGlobalFeatures(const vector<Real>& audio)
    //{
    //    Algorithm* bpmHistogram = AlgorithmFactory::create("RhythmDescriptors");
    //
    //    Real bpm;
    //
    //    vector<Real> vec;
    //
    //    bpmHistogram->input("signal").set(audio);
    //    bpmHistogram->output("bpm").set(bpm);
    //    bpmHistogram->output("beats_position").set("");
    //
    //    bpmHistogram->compute();
    //
    //    vec.push_back(bpm);
    //
    //
    //    delete bpmHistogram;
    //
    //    return vec;
    //}
    
    vector<vector<Real> > Extraction::extractOnsets(const vector<Real>& onsetTimes, const vector<Real>& audio)
    {
        vector<vector<Real> > slices;
        
        //Fix magic SR
        float audioLength = (float)audio.size() / (float)44100.0;
        
        vector<Real>::const_iterator first = onsetTimes.begin()+1;
        vector<Real>::const_iterator last = onsetTimes.end();
        vector<Real> endTimes(first, last);
        endTimes.push_back(audioLength);
        
        Algorithm* slicer = AlgorithmFactory::create("Slicer", "endTimes", endTimes, "startTimes", onsetTimes);
        
        Pool pool;
        
         VectorInput<Real> *v = new VectorInput<Real>();
        v->setVector(&audio);
        *v >> slicer->input("audio");
//        connect(vectorInput->output("data"), slicer->input("signal"));
        slicer->output("frame") >> PC(pool, "frames");
        
        Network n(v);
        n.run();
        

        
//        n.clear();
        
        
        
        vector<vector<Real> > frames;
//        frames =  pool.value<vector<vector<Real> > >("frames");
        
        return frames;
    }
    
    void windowOnsets(vector<vector<Real> >& onsets)
    {
        //Window all the slices
        for(int i=0; i<onsets.size();i++) {
            std::vector<float> hann = Audio::hannWindow(onsets[i].size());
            for(int j=0; j<onsets[i].size(); j++) {
                if(j <= 256)
                    onsets[i][j] = onsets[i][j] * hann[j];
                if(j >= (float)onsets[i].size() / 4.0)
                    onsets[i][j] = onsets[i][j] * hann[j];
            }
        }
    }
    
    inline void Extraction::writeOnsets(const vector<vector<Real> >& slices, const String outputRoot)
    {
        
        for(int i=0; i<slices.size(); i++)
        {
            String fileName = outputRoot + "slice_" + std::to_string(sliceID++) + ".wav";
            Audio::vectorToAudioFile(slices[i], fileName);
        }
    }
    
    Pool Extraction::loadFeatures(const String& jsonFilename)
    {
        Pool pool;
        
        //We get the overall pool, merge and output
        standard::Algorithm* yamlInput  = standard::AlgorithmFactory::create("YamlInput", "format", "json");
        yamlInput->configure("filename", jsonFilename.toStdString());
        yamlInput->output("pool").set(pool);
        yamlInput->compute();

//        std::cout << pool.descriptorNames() << "\n";
        
//        this->globalOnsetPool = pool;
        
        delete yamlInput;
        
        return pool;
    }
    
    
    void Extraction::writeLoop(float onsetTime, const vector<Real>& audio, float BPM, String outFileName)
    {
        float startTimeInSamples = onsetTime * 44100.0;
        
        float lengthOfBeatInSamples = (1.0 / BPM) * 60.0 * 44100.0;
        
        float endTimeInSamples = startTimeInSamples + (lengthOfBeatInSamples * 8.0);
        
        while(endTimeInSamples >= (audio.size() - endTimeInSamples))
            endTimeInSamples = startTimeInSamples + (lengthOfBeatInSamples * 8.0);
        
        vector<Real>::const_iterator first = audio.begin() + (int)startTimeInSamples;
        vector<Real>::const_iterator last = audio.begin() + (int)endTimeInSamples;
        
        vector<Real> newVec(first, last);
        
        Audio::vectorToAudioFile(newVec, outFileName);
    }
    
    vector<Real> Extraction::firstLoop(const vector<Real>& onsetTimes, const vector<Real>& audio, Real BPM, String outFilename)
    {
        float lengthOfBeatInSamples = (1.0 / BPM) * 60.0 * 44100.0;
        
        vector<Real> newVec;
        
        int randomOnsetIndex;
        float startTimeInSamples;
        float endTimeInSamples;
        
        startTimeInSamples = onsetTimes[0] * 44100.0;
        endTimeInSamples = startTimeInSamples + (lengthOfBeatInSamples * 8.0);
        
        //Check yer out of bounds here
        
        if(endTimeInSamples <= audio.size()) {
            vector<Real>::const_iterator first = audio.begin() + (int)startTimeInSamples;
            vector<Real>::const_iterator last = audio.begin() + (int)endTimeInSamples;
            
            newVec = vector<Real>(first, last);
            
            Audio::vectorToAudioFile(newVec, outFilename);
        }
        
        return newVec;
    }
    
    vector<Real> Extraction::randomLoop(const vector<Real>& onsetTimes, const vector<Real>& audio, Real BPM, String outFilename)
    {
        float lengthOfBeatInSamples = (1.0 / BPM) * 60.0 * 44100.0;
        
        vector<Real> newVec;
        
        int randomOnsetIndex;
        float startTimeInSamples;
        float endTimeInSamples;
        
        do  {
            int randomOnsetIndex = random.nextInt(onsetTimes.size());
            startTimeInSamples = onsetTimes[randomOnsetIndex] * 44100.0;
            endTimeInSamples = startTimeInSamples + (lengthOfBeatInSamples * 8.0);
            
        } while(endTimeInSamples > (audio.size()-endTimeInSamples));
        
        vector<Real>::const_iterator first = audio.begin() + (int)startTimeInSamples;
        vector<Real>::const_iterator last = audio.begin() + (int)endTimeInSamples;
        
        newVec = vector<Real>(first, last);
        
        Audio::vectorToAudioFile(newVec, outFilename);
        
        return newVec;
    }
    

    //This batches the files
    Pool Extraction::extractFeaturesFromFiles(const File& audioFolder, bool writeOnsets)
    {
        Pool filePool;
        
        sliceID = 0;
        
        Array<File> filesToProcess = Audio::getAudioFiles(audioFolder);
        
        String outputRoot = audioFolder.getFullPathName() + "/dataset/";
        File datasetDirectory(outputRoot);
        
        datasetDirectory.createDirectory();
    
        File fileNames(outputRoot + "filesProcessed.txt");
        
        vector<Real> labels;
        
        int count = 0;
        
        File filesProcessedFile("/Users/carthach/Desktop/files_juce\n.txt");
        
        for(int i=0; i<filesToProcess.size(); i++) {
            String currentAudioFileName = filesToProcess[i].getFileName();
            
            int label;
            
//            if(currentAudioFileName.startsWith("LO_")) {
//                label = 0;
//                count++;
//            }
//            else if(currentAudioFileName.startsWith("MID_")) {
//                label = 1;
//                count++;
//            }
//            else if(currentAudioFileName.startsWith("HI_")) {
//                label = 2;
//                count++;
//            }
//            else {
//                continue;
//            }
            
            std::cout << "Processing file: " << currentAudioFileName << "\n";
            
//            filesProcessedFile.appendText(currentAudioFileName + "\n");
            
            labels.push_back(label);
            
//            fileNames.appendText(filesToProcess[i].getFileName() + "\n");
            vector<Real> signal = audioTools.audioFileToVector(filesToProcess[i]);
            //        Real BPM =  getGlobalFeatures(signal)[0];
            
            //        std::cout << BPM << "\n";
            
            //------Onset Processing
            
            //Slice
            std::cout << "extractOnsetsTimes" << "\n";
            vector<Real> onsetTimes = extractOnsetTimes(signal);
//            vector<Real> onsetTimes;
            std::cout << "extractOnsets" << "\n";
                vector<vector<Real> > onsetSlices = extractOnsets(onsetTimes, signal);
//            vector<vector<Real> > onsetSlices;
            
            //        vector<vector<Real> > onsetSlices;
            onsetSlices.push_back(signal);
            
            //        std::cout << "noOfOnsets: " << onsetSlices.size() << "\n";
            
            //Loopy stuff - MHD
            //        vector<vector<Real> > loops;
            //        int noOfLoops = 2;
            
            //        for(int j=0; j<noOfLoops; j++) {
            //            loops.push_back(randomLoop(onsetTimes, signal, BPM, outputRoot + String((i*noOfLoops)+j) + "_" + currentAudioFileName +  "_loop_" + String(j) + ".wav"));
            //        }
            
            //        loops.push_back(firstLoop(onsetTimes, signal, BPM, outputRoot + currentAudioFileName +  "_loop.wav"));
            
            //        Pool onsetPool = extractFeatures(loops, BPM);
            //        globalOnsetPool.merge(onsetPool, "append");
            
            //        writeLoop(onsetTimes[5], signal, BPM, outputRoot + "/testy.wav");
            
            
            //Write
            if(writeOnsets)
                this->writeOnsets(onsetSlices, outputRoot);
            
            //Add to pool
            Pool onsetPool = extractFeaturesFromOnsets(onsetSlices, 0);
//
            filePool.merge(onsetPool, "append");
            
            count++;
        }
        
        std::cout << "No. of files processed: " << count << "\n";
        
        filePool.append("labels", labels);
        
        String jsonFilename = outputRoot + "dataset.json";
        
        //We get the overall pool, merge and output
        standard::Algorithm* yamlOutput  = standard::AlgorithmFactory::create("YamlOutput", "format", "json", "writeVersion", false);
        yamlOutput->input("pool").set(filePool);
        yamlOutput->configure("filename", jsonFilename.toStdString());
        yamlOutput->compute();
        
        //    File jsonFile(jsonFilename);
        //    String jsonFileText = jsonFile.loadFileAsString();
        //    jsonFileText = "%YAML:1.0\n" + jsonFileText;
        //    jsonFile.replaceWithText(jsonFileText);
        
        //    cv::Mat erbHi = poolToMat(pool);      
        
//        cv::Mat poolMat = globalPoolToMat();
        
        //    cv::Mat pcaOut = pcaReduce(poolMat, 3);
        return filePool;
    }
    
    StringArray Extraction::featuresInPool(const Pool& pool)
    {
        StringArray featureList;
        
        RealMap realMap = pool.getRealPool();
        VectorMap vectorMap = pool.getVectorRealPool();
        
        for(RealMapIter iterator = realMap.begin(); iterator != realMap.end(); iterator++)
            featureList.add(iterator->first);
        
        for(VectorMapIter iterator = vectorMap.begin(); iterator != vectorMap.end(); iterator++) {
            for(int i=0; i<iterator->second[0].size(); i++)
                featureList.add(iterator->first + "_" + String(i));
        }
        
        return featureList;
    }
    
    Pool Extraction::extractFeatures(vector<Real>& audio, Real BPM)
    {
        network->reset();
        
        framePool.clear();
        
        vectorInput->setVector(&audio);
//        network->run();
        
//        network->run();
        
        // aggregate the results
        Pool aggrPool; // the pool with the aggregated MFCC values
        
        poolAggregator->input("input").set(framePool);
        poolAggregator->output("output").set(aggrPool);
        poolAggregator->compute();
        
        //For merging pools together the JSON entries need to be vectorised (using .add function)
        //aggrPool gets rid of this, so a fix is to remove and add again
        
        //Reals
        std::map< std::string, Real>  reals = aggrPool.getSingleRealPool();
        typedef std::map<std::string, Real>::iterator realsIterator;
        
        for(realsIterator iterator = reals.begin(); iterator != reals.end(); iterator++) {
            aggrPool.remove(iterator->first);
            aggrPool.add(iterator->first, iterator->second);
        }
        
        
        //        //Compute and add the global features
        //        zcr->reset();
        //        zcr->input("signal").set(*sliceIterator);
        //        zcr->compute();
        //
        //        lat->reset();
        //        lat->input("signal").set(*sliceIterator);
        //        lat->compute();
        //
        //        envelope->reset();
        //        envelope->input("signal").set(*sliceIterator);
        //        envelope->compute();
        //
        //        tct->reset();
        //        tct->compute();
        //
        //        aggrPool.add("zcr", zcrReal);
        //        aggrPool.add("lat", latReal);
        //        aggrPool.add("tct", tctReal);

        
//        aggrPool.add("RMS", rmsValue);
        
        //Get the mean of the erbBands to get lo/mid/hi
        
        std::map< std::string, std::vector<Real > >  vectors = aggrPool.getRealPool();
        
        vector<Real> aggrBands = vectors["bands.mean"];

        
        Algorithm* mean = AlgorithmFactory::create("Mean");
        
        
//        //=========== ERB STUFF =========
//        //Get erbLo
//        vector<Real>::const_iterator first = aggrBands.begin();
//        vector<Real>::const_iterator last = aggrBands.begin() + 2;
//        vector<Real> loBands(first, last);
//        
//        Real loValue;
//        mean->input("array").set(loBands);
//        mean->output("mean").set(loValue);
//        mean->compute();
//        aggrPool.add("loValue", loValue);
//        
//        //Get erbMid
//        first = aggrBands.begin()+2;
//        last = aggrBands.begin()+5;
//        vector<Real> midBands(first, last);
//        
//        Real midValue;
//        
//        mean->reset();
//        mean->input("array").set(midBands);
//        mean->output("mean").set(midValue);
//        mean->compute();
//        aggrPool.add("midValue", midValue);
//        
//        //Get erbHi
//        first = aggrBands.begin()+5;
//        //        last = aggrBands.end();
//        last = aggrBands.begin()+10;
//        vector<Real> hiBands(first, last);
//        
//        Real hiValue;
//        
//        mean->reset();
//        mean->input("array").set(hiBands);
//        mean->output("mean").set(hiValue);
//        mean->compute();
//        aggrPool.add("hiValue", hiValue);
//        
//        //Remove the original full erbBand vectors
//        aggrPool.remove("bands.mean");
//        aggrPool.remove("bands.var");
        
        //Remove/Add mfcc vector
        vector<Real> aggrBandsMean = vectors["mfcc.mean"];
        vector<Real> aggrBandsVar = vectors["mfcc.var"];
        aggrPool.remove("mfcc.mean");
        aggrPool.remove("mfcc.var");
        aggrPool.add("mfcc.mean", aggrBandsMean);
        aggrPool.add("mfcc.var", aggrBandsVar);
        
//        aggrPool.add("BPM", BPM);
        
        //If you want to output individual aggregate pools
        //            if(outputAggrPool) {
        //                yamlOutput->reset();
        //                yamlOutput->input("pool").set(aggrPool);
        //                string fileName = "slice_" + std::to_string(localSliceID++) + ".yaml";
        //                yamlOutput->configure("filename", fileName);
        //                yamlOutput->compute();
        //            }
        
        //Finally do the merge to the overall onsetPool
        return aggrPool;
    }
    
    //This batches the onsets
    Pool Extraction::extractFeaturesFromOnsets(vector<vector<Real> >& slices, Real BPM)
    {
        Pool onsetsPool;
        
        //Loop through all the slices
        vector<vector<Real> >::iterator sliceIterator;
        
        //Go through every onset and extract the features
        for(sliceIterator = slices.begin(); sliceIterator != slices.end(); sliceIterator++)
        {
            Pool onsetPool = extractFeatures(*sliceIterator, 0);
            onsetsPool.merge(onsetPool, "append");
        }

        //Remove unwanted stuff
        onsetsPool.remove("BPM");
        onsetsPool.remove("RMS");
        onsetsPool.remove("flatness.mean");
        onsetsPool.remove("flatness.var");
        onsetsPool.remove("pitch.mean");
        onsetsPool.remove("pitch.var");
        onsetsPool.remove("spectral_centroid.mean");
        onsetsPool.remove("spectral_centroid.var");
        //    std::cout << onsetPool.descriptorNames();
        
        return onsetsPool;
    }
}