/*********************************************************************
*
* Software License Agreement (BSD License)
*
*  Copyright (c) 2015, P.A.N.D.O.R.A. Team.
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the P.A.N.D.O.R.A. Team nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*
* Authors:
*   Kofinas Miltiadis <mkofinas@gmail.com>
*   Protopapas Marios <protopapas_marios@hotmail.com>
*********************************************************************/

#include <vector>

#include <boost/shared_ptr.hpp>

#include "pandora_vision_victim/feature_extractors/depth_feature_extraction.h"
#include "pandora_vision_victim/feature_extractors/sift_extractor.h"
#include "pandora_vision_victim/feature_extractors/hog_extractor.h"
#include "pandora_vision_victim/utilities/file_utilities.h"
#include "pandora_vision_victim/utilities/bag_of_words_trainer.h"

/**
 * @namespace pandora_vision
 * @brief The main namespace for PANDORA vision
 */
namespace pandora_vision
{
  /**
   * @brief Default Constructor
   */
  DepthFeatureExtraction::DepthFeatureExtraction() : FeatureExtraction()
  {
    bool extractChannelsStatisticsFeatures = true;
    bool extractEdgeOrientationFeatures = true;
    bool extractHaralickFeatures = true;
    bool extractSiftFeatures = false;
    bool extractHogFeatures = false;

    chosenFeatureTypesMap_["channels_statistics"] =
        extractChannelsStatisticsFeatures;
    chosenFeatureTypesMap_["edge_orientation"] =
        extractEdgeOrientationFeatures;
    chosenFeatureTypesMap_["haralick"] = extractHaralickFeatures;
    chosenFeatureTypesMap_["sift"] = extractSiftFeatures;
    chosenFeatureTypesMap_["hog"] = extractHogFeatures;

    if (chosenFeatureTypesMap_["sift"] == true)
    {
      boost::shared_ptr<FeatureExtractorFactory> siftPtr(new SiftExtractor());
      featureFactoryPtrMap_["sift"] = siftPtr;

      bowTrainerPtr_.reset(new BagOfWordsTrainer());
    }
    if (chosenFeatureTypesMap_["hog"] == true)
    {
      boost::shared_ptr<FeatureExtractorFactory> hogPtr(new HOGExtractor());
      featureFactoryPtrMap_["hog"] = hogPtr;
    }
  }

  /**
   * @brief Default Destructor
   */
  DepthFeatureExtraction::~DepthFeatureExtraction()
  {
  }

  /**
   * @brief This function extracts features from Depth images according to
   * a predefined set of feature extraction algorithms.
   * @param inImage [const cv::Mat&] RGB frame to extract features from.
   * @return void
   */
  void DepthFeatureExtraction::extractFeatures(const cv::Mat& inImage)
  {
    /// Clear feature vector
    if (!featureVector_.empty())
      featureVector_.clear();
    if (chosenFeatureTypesMap_["channels_statistics"] == true)
    {
      /// Extract Color Statistics features from Depth image
      std::vector<double> channelsStatisticsFeatureVector;
      ChannelsStatisticsExtractor::findDepthChannelsStatisticsFeatures(inImage,
          &channelsStatisticsFeatureVector);
      /// Append Color Statistics features to Depth feature vector.
      for (int ii = 0; ii < channelsStatisticsFeatureVector.size(); ii++)
        featureVector_.push_back(channelsStatisticsFeatureVector[ii]);
    }

    if (chosenFeatureTypesMap_["edge_orientation"] == true)
    {
      /// Extract Edge Orientation features from Depth image
      std::vector<double> edgeOrientationFeatureVector;
      EdgeOrientationExtractor::findEdgeFeatures(inImage,
          &edgeOrientationFeatureVector);
      /// Append Edge Orientation features to Depth feature vector.
      for (int ii = 0; ii < edgeOrientationFeatureVector.size(); ii++)
        featureVector_.push_back(edgeOrientationFeatureVector[ii]);
    }

    if (chosenFeatureTypesMap_["haralick"] == true)
    {
      /// Extract Haralick features from Depth image
      std::vector<double> haralickFeatureVector;
      HaralickFeaturesExtractor::findHaralickFeatures(inImage,
          &haralickFeatureVector);
      /// Append Haralick features to Depth feature vector.
      for (int ii = 0; ii < haralickFeatureVector.size(); ii++)
        featureVector_.push_back(haralickFeatureVector[ii]);
    }

    if (chosenFeatureTypesMap_["sift"] == true)
    {
      /// Extract SIFT features from RGB image
      cv::Mat siftDescriptors;
      bowTrainerPtr_->createBowRepresentation(inImage, &siftDescriptors);
      /// Append SIFT features to RGB feature vector.
      for (int ii = 0; ii < siftDescriptors.cols; ii++)
        featureVector_.push_back(siftDescriptors.at<float>(ii));
    }

    if (chosenFeatureTypesMap_["hog"] == true)
    {
      /// Extract HOG features from RGB image
      std::vector<float> hogDescriptors;
      featureFactoryPtrMap_["hog"]->extractFeatures(inImage, &hogDescriptors);
      /// Append HOG features to RGB feature vector.
      for (int ii = 0; ii < hogDescriptors.size(); ii++)
        featureVector_.push_back(hogDescriptors.at(ii));
    }

  }
}  // namespace pandora_vision
