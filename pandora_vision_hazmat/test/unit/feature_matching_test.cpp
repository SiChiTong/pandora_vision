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
 * Authors: Choutas Vassilis 
 *********************************************************************/


#include "pandora_vision_hazmat/mock/mock_feature_detector.h"
#include "gtest/gtest.h"


class FeatureMatchingDetectorTest : public ::testing::Test 
{
  public:
    MockFeatureDetector MockDetector;
  private:
};

/* Check if the different functions of the feature based detector
 * behave correctly when they are given wrong input,that is they do not
 * perform any calculation and return a false flag to signify the problem.
 */
TEST_F(FeatureMatchingDetectorTest, test_input)
{
  bool testFlag;

  cv::Mat emptyMat;
  emptyMat.data = NULL;

  cv::Mat identityMat = cv::Mat::ones(3, 3, CV_32FC1);
  std::vector<cv::KeyPoint> emptyVec;
  std::vector<cv::KeyPoint> testVecKeypoint;
  std::vector<cv::Point2f> emptyVecP2f;
  std::vector<cv::Point2f> testVecP2f;
  std::vector<cv::Point2f> returnVec;

  testVecKeypoint.push_back(cv::KeyPoint());
  testVecKeypoint.push_back(cv::KeyPoint());
  testVecKeypoint.push_back(cv::KeyPoint());
  testVecKeypoint.push_back(cv::KeyPoint());
  
  testVecP2f.push_back(cv::Point2f());
  testVecP2f.push_back(cv::Point2f());
  testVecP2f.push_back(cv::Point2f());
  testVecP2f.push_back(cv::Point2f());

  // Check that if the array containing the descriptors calculated
  // from the frame is empty then the function returns false.
  testFlag = MockDetector.findKeypointMatches(emptyMat, identityMat ,
      testVecP2f, testVecKeypoint, &returnVec, &returnVec);
  ASSERT_FALSE(testFlag);
  
  // This time an empty array of the pattern descriptors is passed to the function.
  testFlag = MockDetector.findKeypointMatches(identityMat, emptyMat ,
      testVecP2f, testVecKeypoint, &returnVec, &returnVec);
  ASSERT_FALSE(testFlag);
// Pass an empty vector of pattern testVecKeypoint. The function must return false and not perform any calculation.
  testFlag = MockDetector.findKeypointMatches(identityMat, identityMat ,
      emptyVecP2f, testVecKeypoint, &returnVec, &returnVec);
  ASSERT_FALSE(testFlag);

  // Pass an empty vector of scene testVecKeypoint. The function must not do any computation and return a false flag.
  testFlag = MockDetector.findKeypointMatches(identityMat, identityMat ,
      testVecP2f, emptyVec, &returnVec, &returnVec);
  ASSERT_FALSE(testFlag);
  
  returnVec.push_back(cv::Point2f(1, 0));
  testFlag = MockDetector.findBoundingBox(emptyVecP2f, returnVec, returnVec, 
      &returnVec );
  ASSERT_FALSE(testFlag);
  testFlag = MockDetector.findBoundingBox(returnVec, emptyVecP2f, returnVec,
      &returnVec );
  ASSERT_FALSE(testFlag);
  testFlag = MockDetector.findBoundingBox(returnVec, returnVec, emptyVecP2f,
      &returnVec );
  ASSERT_FALSE(testFlag);

}
