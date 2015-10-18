/*
  ==============================================================================

    Information.cpp
    Created: 18 Oct 2015 11:58:19am
    Author:  Cárthach Ó Nuanáin

  ==============================================================================
*/

#include "Information.h"


namespace Muce {
    Information::Information()
    {
        
    }
    Information::~Information()
    {
        
    }
    
    
    cv::Mat Information::knnTrain(essentia::Pool pool)
    {
        //Train Classifier
        if (pool.contains<std::vector<essentia::Real> >("labels")) {
            std::vector<essentia::Real> labelsVector = pool.value<std::vector<essentia::Real> >("labels");
            pool.remove("labels");
            
            cv::Mat features = poolToMat(pool);
            cv::Mat labels(labelsVector, true);
            
            knn.train(features, labels);
        }        
    }
    
    cv::Mat Information::poolToMat(const essentia::Pool& pool)
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
    
    cv::Mat Information::kMeans(cv::Mat points, int k)
    {
        using namespace cv;
        cv::Mat labels, centers;
        
        kmeans(points, k, labels,
               TermCriteria( TermCriteria::EPS+TermCriteria::COUNT, 10, 1.0),
               3, KMEANS_PP_CENTERS, centers);
        
        return labels;
    }
    
    cv::Mat Information::knnClassify(cv::Mat instances, int k)
    {
        cv::Mat results;
        knn.find_nearest(instances, k, &results, 0, 0, 0);
        
        return results;
    }
    
    /* Use openCV and PCA to collapse the MFCCs to 2D points for visualisation */
    
    cv::Mat Information::pcaReduce(cv::Mat mat, int noOfDimensions)
    {
        cv::Mat projection_result;
        
        cv::PCA pca(mat,cv::Mat(),CV_PCA_DATA_AS_ROW, noOfDimensions);
        
        pca.project(mat,projection_result);
        
        return projection_result;
    }
    
    /* Use openCV and PCA to collapse the MFCCs to 2D points for visualisation */
    
    void Information::normaliseFeatures(cv::Mat mat)
    {
        for(int i=0; i <mat.cols; i++) {
            cv::normalize(mat.col(i), mat.col(i), 0, 1, cv::NORM_MINMAX, CV_32F);
        }
    }
    
    cv::Mat Information::globalPoolToMat()
    {
//        return poolToMat(globalOnsetPool);
    }
    
    void Information::readYamlToMatrix(const String& yamlFilename, const StringArray& featureList)
    {
        using namespace cv;
        
        FileStorage::FileStorage fs2(yamlFilename.toStdString(), FileStorage::READ);
        
        FileNode features = fs2["erbHi"];
        
        std::cout << (float)features[0];
        
        
        //    // first method: use (type) operator on FileNode.
        //    int frameCount = (int)fs2["frameCount"];
        //
        //    std::string date;
        //    // second method: use FileNode::operator >>
        //    fs2["calibrationDate"] >> date;
        //
        //    Mat cameraMatrix2, distCoeffs2;
        //    fs2["cameraMatrix"] >> cameraMatrix2;
        //    fs2["distCoeffs"] >> distCoeffs2;
        //
        //    cout << "frameCount: " << frameCount << endl
        //    << "calibration date: " << date << endl
        //    << "camera matrix: " << cameraMatrix2 << endl
        //    << "distortion coeffs: " << distCoeffs2 << endl;
        //
        //    FileNode features = fs2["features"];
        //    FileNodeIterator it = features.begin(), it_end = features.end();
        //    int idx = 0;
        //    std::vector<uchar> lbpval;
        //    
        //    // iterate through a sequence using FileNodeIterator
        //    for( ; it != it_end; ++it, idx++ )
        //    {
        //        cout << "feature #" << idx << ": ";
        //        cout << "x=" << (int)(*it)["x"] << ", y=" << (int)(*it)["y"] << ", lbp: (";
        //        // you can also easily read numerical arrays using FileNode >> std::vector operator.
        //        (*it)["lbp"] >> lbpval;
        //        for( int i = 0; i < (int)lbpval.size(); i++ )
        //            cout << " " << (int)lbpval[i];
        //        cout << ")" << endl;
        //    }
        
        fs2.release();
    }
    
    void clusterData(cv::Mat data)
    {
        cv::Mat labels;
        cv::kmeans(data, 3, labels, cv::TermCriteria( cv::TermCriteria::EPS+cv::TermCriteria::COUNT, 10, 1.0), 3, cv::KMEANS_PP_CENTERS, cv::noArray());
    }
}