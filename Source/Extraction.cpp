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

    }
    
    void Extraction::initAlgorithms()
    {
        AlgorithmFactory& factory = standard::AlgorithmFactory::instance();
        
        //Stastical things
        float halfSampleRate = (float)sampleRate / 2.0;
        
        
        frameCutter    = factory.create("FrameCutter",
                                        "frameSize", frameSize,
                                        "hopSize", hopSize);
        window     = factory.create("Windowing", "type", "hann");
        
        spec  = factory.create("Spectrum");
        
        mfcc  = factory.create("MFCC");
        
        centroid = factory.create("Centroid", "range", halfSampleRate);
        centralMoments = factory.create("CentralMoments", "range", halfSampleRate);
        distShape = factory.create("DistributionShape");
        
        spectralFlatness = factory.create("FlatnessDB");
        
        //Energy Bands
        //    Algorithm* bands = factory.create("ERBBands");
        bands = factory.create("ERBBands");
        
        //Global
        RMS = factory.create("RMS");
        zcr = factory.create("ZeroCrossingRate");
        lat = factory.create("LogAttackTime");
        envelope = factory.create("Envelope");
        tct = factory.create("TCToTotal");
        
        pitch = factory.create("PitchYinFFT");

        
        //Yaml Output in JSON format
        //Aggregate Pool stats
        string defaultStats[] = {"mean", "var"};
        poolAggregator = factory.create("PoolAggregator", "defaultStats", arrayToVector<string>(defaultStats));
        
        yamlOutput  = factory.create("YamlOutput", "format", "yaml");
        
        

        
        frameCutter->output("frame").set(frame);
        window->input("frame").set(frame);
        
        window->output("frame").set(windowedFrame);
        spec->input("frame").set(windowedFrame);
        

        
        spec->output("spectrum").set(spectrum);
        mfcc->input("spectrum").set(spectrum);
        
        mfcc->output("bands").set(mfccBands);
        mfcc->output("mfcc").set(mfccCoeffs);
        

        
        bands->input("spectrum").set(spectrum);
        bands->output("bands").set(bandsVector);
        

        centroid->input("array").set(spectrum);
        centroid->output("centroid").set(spectralCentroid);
        
        pitch->input("spectrum").set(spectrum);
        pitch->output("pitch").set(pitchReal);
        pitch->output("pitchConfidence").set(pitchConfidence);
        pitch->output("pitch").set(pitchReal);
        

        spectralFlatness->input("array").set(spectrum);
        spectralFlatness->output("flatnessDB").set(spectralFlatnessReal);
        
        //Central Moments

        centralMoments->input("array").set(spectrum);
        centralMoments->output("centralMoments").set(moments);
        

        distShape->input("centralMoments").set(moments);
        distShape->output("spread").set(spread);
        distShape->output("skewness").set(skewness);
        distShape->output("kurtosis").set(kurtosis);
        
        //Global stats (don't set input until in the loop)

        zcr->output("zeroCrossingRate").set(zcrReal);
        

        lat->output("logAttackTime").set(latReal);
        

        envelope->output("signal").set(envelopeSignal);
        

        tct->input("envelope").set(envelopeSignal);
        tct->output("TCToTotal").set(tctReal);
        
        
        
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
        Algorithm* extractoronsetrate = AlgorithmFactory::create("OnsetRate");
        
        Real onsetRate;
        vector<Real> onsets;
        
        extractoronsetrate->input("signal").set(audio);
        extractoronsetrate->output("onsets").set(onsets);
        //    extractoronsetrate->configure("ratioThreshold", 12.0);
        //    extractoronsetrate->configure("combine", 100.0);
        extractoronsetrate->output("onsetRate").set(onsetRate);
        
        extractoronsetrate->compute();
        
        delete extractoronsetrate;
        
        return onsets;
    }
    
    
    
    vector<Real> Extraction::extractRhythmFeatures(const vector<Real>& audio)
    {
        Algorithm* rhythmExtractor = AlgorithmFactory::create("RhythmExtractor2013", "method", "degara");
        
        Real bpm ,confidence;
        vector<Real> ticks, estimates, bpmIntervals;
        
        rhythmExtractor->input("signal").set(audio);
        rhythmExtractor->output("bpm").set(bpm);
        rhythmExtractor->output("ticks").set(ticks);
        rhythmExtractor->output("estimates").set(estimates);
        rhythmExtractor->output("bpmIntervals").set(bpmIntervals);
        rhythmExtractor->output("confidence").set(confidence);
        
        rhythmExtractor->compute();
        
        vector<Real> blah;
        blah.push_back(bpm);
        
        delete rhythmExtractor;
        
        return blah;
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
        AlgorithmFactory& factory = standard::AlgorithmFactory::instance();
        
        //Fix magic SR
        float audioLength = (float)audio.size() / (float)44100.0;
        
        vector<Real>::const_iterator first = onsetTimes.begin()+1;
        vector<Real>::const_iterator last = onsetTimes.end();
        vector<Real> endTimes(first, last);
        endTimes.push_back(audioLength);
        
        Algorithm* slicer = factory.create("Slicer",
                                           "endTimes", endTimes,
                                           "startTimes",
                                           onsetTimes);
        
        slicer->input("audio").set(audio);
        slicer->output("frame").set(slices);
        slicer->compute();
        
        delete slicer;
        
        for(int i=0; i<slices.size();i++) {
            std::vector<float> hann = Audio::hannWindow(slices[i].size());
            for(int j=0; j<slices[i].size(); j++) {
                if(j <= 256)
                    slices[i][j] = slices[i][j] * hann[j];
                if(j >= (float)slices[i].size() / 4.0)
                    slices[i][j] = slices[i][j] * hann[j];
            }
        }
        
        return slices;
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
        Algorithm* yamlInput  = AlgorithmFactory::create("YamlInput", "format", "json");
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
    

    
    Pool Extraction::extractFeaturesFromFolder(const File& audioFolder, bool writeOnsets)
    {
        Pool folderPool;
        
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
            vector<Real> signal = audio.audioFileToVector(filesToProcess[i]);
            //        Real BPM =  getGlobalFeatures(signal)[0];
            
            //        std::cout << BPM << "\n";
            
            //------Onset Processing
            
            //Slice
            vector<Real> onsetTimes = extractOnsetTimes(signal);
            vector<vector<Real> > onsetSlices = extractOnsets(onsetTimes, signal);
            
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
            folderPool.merge(onsetPool, "append");
        }
        
        std::cout << "No. of files processed: " << count << "\n";
        
        folderPool.append("labels", labels);
        
        String jsonFilename = outputRoot + "dataset.json";
        
        //We get the overall pool, merge and output
        Algorithm* yamlOutput  = standard::AlgorithmFactory::create("YamlOutput", "format", "json", "writeVersion", false);
        yamlOutput->input("pool").set(folderPool);
        yamlOutput->configure("filename", jsonFilename.toStdString());
        yamlOutput->compute();
        
        //    File jsonFile(jsonFilename);
        //    String jsonFileText = jsonFile.loadFileAsString();
        //    jsonFileText = "%YAML:1.0\n" + jsonFileText;
        //    jsonFile.replaceWithText(jsonFileText);
        
        //    cv::Mat erbHi = poolToMat(pool);      
        
//        cv::Mat poolMat = globalPoolToMat();
        
        //    cv::Mat pcaOut = pcaReduce(poolMat, 3);
        return folderPool;
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
    
    Pool Extraction::extractFeatures(const vector<Real>& audio, Real BPM)
    {
        Pool framePool, aggrPool;
        
        //Reset the framecutter and set to the new input
        frameCutter->reset();
        frameCutter->input("signal").set(audio);
        
        //Reset the framewise algorithms
        window->reset();
        spec->reset();
        mfcc->reset();
        centroid->reset();
        centralMoments->reset();
        distShape->reset();
        
        //Reset MHD descriptors
        spectralFlatness->reset();
        pitch->reset();
        bands->reset();
        
        poolAggregator->reset();
        poolAggregator->input("input").set(framePool);
        poolAggregator->output("output").set(aggrPool);
        
        framePool.clear();
        
        //Start the frame cutter
        while (true) {
            
            // compute a frame
            frameCutter->compute();
            
            // if it was the last one (ie: it was empty), then we're done.
            if (!frame.size()) {
                break;
            }
            
            // if the frame is silent, just drop it and go on processing
            if (isSilent(frame)) continue;
            
            //Spectrum and MFCC
            window->compute();
            spec->compute();
            
            //            //MFCC
            //            mfcc->compute();
            //            framePool.add("mfcc",mfccCoeffs);
            
            centroid->compute();
            
            framePool.add("spectral_centroid", spectralCentroid);
            
            bands->compute();
            framePool.add("bands", bandsVector);
            
            centralMoments->compute();
            distShape->compute();
            
            //MHD
            pitch->compute();
            framePool.add("pitch", pitchReal);
            
            spectralFlatness->compute();
            framePool.add("flatness", spectralFlatnessReal);
            
            //            framePool.add("spectral_spread", spread);
            //            framePool.add("spectral_skewness", skewness);
            //            framePool.add("spectral_kurtosis", kurtosis);
        }
        
        //Time to aggregate
        aggrPool.clear();
        poolAggregator->reset();
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
        
        Real rmsValue;
        RMS->reset();
        RMS->input("array").set(audio);
        RMS->output("rms").set(rmsValue);
        RMS->compute();
        
        aggrPool.add("RMS", rmsValue);
        
        
        //Get the mean of the erbBands to get lo/mid/hi
        
        std::map< std::string, std::vector<Real > >  vectors = aggrPool.getRealPool();
        
        vector<Real> aggrBands = vectors["bands.mean"];

        
        Algorithm* mean = AlgorithmFactory::create("Mean");
        
        
        //=========== ERB STUFF =========
        //Get erbLo
        vector<Real>::const_iterator first = aggrBands.begin();
        vector<Real>::const_iterator last = aggrBands.begin() + 2;
        vector<Real> loBands(first, last);
        
        Real loValue;
        mean->input("array").set(loBands);
        mean->output("mean").set(loValue);
        mean->compute();
        aggrPool.add("loValue", loValue);
        
        //Get erbMid
        first = aggrBands.begin()+2;
        last = aggrBands.begin()+5;
        vector<Real> midBands(first, last);
        
        Real midValue;
        
        mean->reset();
        mean->input("array").set(midBands);
        mean->output("mean").set(midValue);
        mean->compute();
        aggrPool.add("midValue", midValue);
        
        //Get erbHi
        first = aggrBands.begin()+5;
        //        last = aggrBands.end();
        last = aggrBands.begin()+10;
        vector<Real> hiBands(first, last);
        
        Real hiValue;
        
        mean->reset();
        mean->input("array").set(hiBands);
        mean->output("mean").set(hiValue);
        mean->compute();
        aggrPool.add("hiValue", hiValue);
        
        //Remove the original full erbBand vectors
        aggrPool.remove("bands.mean");
        aggrPool.remove("bands.var");
        
        //        //Remove/Add mfcc vector
        //        vector<Real> aggrBandsMean = vectors["mfcc.mean"];
        //        vector<Real> aggrBandsVar = vectors["mfcc.var"];
        //        aggrPool.remove("mfcc.mean");
        //        aggrPool.remove("mfcc.var");
        //        aggrPool.add("mfcc.mean", aggrBandsMean);
        //        aggrPool.add("mfcc.var", aggrBandsVar);
        
        aggrPool.add("BPM", BPM);
        
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
    
    //Put your extractor code here
    Pool Extraction::extractFeaturesFromOnsets(vector<vector<Real> >& slices, Real BPM)
    {
        
        //The 3 levels of pools
        Pool onsetPool;
        

        
        //Loop through all the slices
        vector<vector<Real> >::iterator sliceIterator;
        
        //Go through every onset and extract the features
        for(sliceIterator = slices.begin(); sliceIterator != slices.end(); sliceIterator++)
        {
            Pool onsetFeatures = extractFeatures(*sliceIterator, 0);
            onsetPool.merge(onsetFeatures, "append");
        }

        //Remove unwanted stuff
        onsetPool.remove("BPM");
        onsetPool.remove("RMS");
        onsetPool.remove("flatness.mean");
        onsetPool.remove("flatness.var");
        onsetPool.remove("pitch.mean");
        onsetPool.remove("pitch.var");
        onsetPool.remove("spectral_centroid.mean");
        onsetPool.remove("spectral_centroid.var");
        //    std::cout << onsetPool.descriptorNames();
        
        return onsetPool;
    }
}