/*********************************************************************
 *
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2014, P.A.N.D.O.R.A. Team.
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
 * Authors: Alexandros Philotheou, Manos Tsardoulias
 *********************************************************************/

#ifndef HOLE_FUSION_NODE_HOLE_FUSION_H
#define HOLE_FUSION_NODE_HOLE_FUSION_H

#include <dirent.h>
#include <ros/package.h>
#include <std_msgs/Empty.h>
#include "vision_communications/CandidateHolesVectorMsg.h"
#include "vision_communications/CandidateHoleMsg.h"
#include "utils/defines.h"
#include "utils/histogram_calculation.h"
#include "utils/message_conversions.h"
#include "utils/noise_elimination.h"
#include "utils/parameters.h"
#include "utils/visualization.h"
#include "hole_fusion_node/depth_filters.h"
#include "hole_fusion_node/filters_resources.h"
#include "hole_fusion_node/rgb_filters.h"
#include "hole_fusion_node/hole_merger.h"

/**
  @namespace vision
  @brief The main namespace for PANDORA vision
 **/
namespace pandora_vision
{
  class HoleFusion
  {
    private:

      //!< The ROS node handle
      ros::NodeHandle nodeHandle_;

      //!< The ROS publisher that will be used for unlocking the
      //!< synchronizer_node
      ros::Publisher unlockPublisher_;

      //!< The ROS subscriber for acquisition of candidate holes originated
      //!< from the depth node
      ros::Subscriber depthCandidateHolesSubscriber_;

      //!< The ROS subscriber for acquisition of candidate holes originated
      //!< from the rgb node
      ros::Subscriber rgbCandidateHolesSubscriber_;

      //!< The ROS subscriber for acquisition of the point cloud originated
      //!< from th e synchronizer node
      ros::Subscriber pointCloudSubscriber_;

      //!< Indicates how many of the depth_node and rgb_node nodes have
      //!< received hole candidates and are ready to send them for processing
      int numNodesReady_;

      //!< The rgb received by the RGB node
      cv::Mat rgbImage_;

      //!< The point cloud received by the depth node
      PointCloudXYZPtr pointCloudXYZ_;

      //!< The interpolated depth image received by the depth node
      cv::Mat interpolatedDepthImage_;

      //!< The conveyor of hole candidates received by the depth node
      HolesConveyor depthHolesConveyor_;

      //!< The conveyor of hole candidates received by the rgb node
      HolesConveyor rgbHolesConveyor_;

      //!< A histogramm for the texture of walls
      cv::MatND wallsHistogram_;

      //!< The dynamic reconfigure (hole fusion's) parameters' server
      dynamic_reconfigure::Server<pandora_vision_hole_detector::
        hole_fusion_cfgConfig> server;

      //!< The dynamic reconfigure (hole fusion's) parameters' callback
      dynamic_reconfigure::Server<pandora_vision_hole_detector::
        hole_fusion_cfgConfig>:: CallbackType f;


      /**
        @brief Callback for the candidate holes via the depth node
        @param[in] depthCandidateHolesVector
        [const vision_communications::CandidateHolesVectorMsg&]
        The message containing the necessary information to filter hole
        candidates acquired through the depth node
        @return void
       **/
      void depthCandidateHolesCallback(
        const vision_communications::CandidateHolesVectorMsg&
        depthCandidateHolesVector);

      /**
        @brief Recreates the HolesConveyor struct for the
        candidate holes from the
        vision_communications::CandidateHolerMsg message
        @param[in]candidateHolesVector
        [const std::vector<vision_communications::CandidateHoleMsg>&]
        The input candidate holes
        @param[out] conveyor [HolesConveyor*] The output conveyor
        struct
        @return void
       **/
      void fromCandidateHoleMsgToConveyor(
        const std::vector<vision_communications::CandidateHoleMsg>&
        candidateHolesVector,
        HolesConveyor* conveyor);

      /**
        @brief With an input a conveyor of holes, this method, depending on
        the depth image's interpolation method, has holes assimilating,
        amalgamating or being connected with holes that can be assimilated,
        amalgamated or connected with or by them. The interpolation method is
        a basic criterion for the mode of merging holes because the two
        filters that verify the validity of each merger are depth-based ones.
        If there is no valid depth image on which to run these filters, it is
        sure that the depth sensor is closer to the scene it is witnessing
        than 0.5-0.6m. In this way of operation, the merging of holes does not
        consider employing validator filters and simply merges holes that can
        be merged with each other (assimilated, amalgamated, or connected).
        @param[in out] conveyor [HolesConveyor*] The conveyor of holes to be
        merged with one another, where applicable.
        @return void
       **/
      void mergeHoles(HolesConveyor* conveyor);

      /**
        @brief The function called when a parameter is changed
        @param[in] config
        [const pandora_vision_hole_detector::hole_fusion_cfgConfig&]
        @param[in] level [const uint32_t] The level (?)
        @return void
       **/
      void parametersCallback(
        const pandora_vision_hole_detector::hole_fusion_cfgConfig& config,
        const uint32_t& level);

      /**
        @brief Callback for the point cloud that the synchronizer node
        publishes
        @param[in] msg [const sensor_msgs::PointCloud2ConstPtr&] The message
        containing the point cloud
        @return void
       **/
      void pointCloudCallback(const sensor_msgs::PointCloud2ConstPtr& msg);

      /**
        @brief Implements a strategy to combine
        information from both sources in order to accurately find valid holes
        @return void
       **/
      void processCandidateHoles();

      /**
        @brief Callback for the candidate holes via the rgb node
        @param[in] depthCandidateHolesVector
        [const vision_communications::CandidateHolesVectorMsg&]
        The message containing the necessary information to filter hole
        candidates acquired through the rgb node
        @return void
       **/
      void rgbCandidateHolesCallback(
        const vision_communications::CandidateHolesVectorMsg&
        rgbCandidateHolesVector);

      /**
        @brief Sets the depth values of a point cloud according to the
        values of a depth image
        @param[in] inImage [const cv::Mat&] The depth image in CV_32FC1 format
        @param[out] pointCloudXYZPtr [PointCloudXYZPtr*] The point cloud
        @return void
       **/
      void setDepthValuesInPointCloud(const cv::Mat& inImage,
        PointCloudXYZPtr* pointCloudXYZPtr);

      /**
        @brief Runs candidate holes through selected filters.
        Probabilities for each candidate hole and filter
        are printed in the console, with an order specified by the
        hole_fusion_cfg of the dynamic reconfigure utility
        @param[in] conveyor [const HolesConveyor&] The conveyor
        containing candidate holes
        @return void
       **/
      void siftHoles(const HolesConveyor& conveyor);

      /**
        @brief Requests from the synchronizer to process a new point cloud
        @return void
       **/
      void unlockSynchronizer();

      /**
        @brief Unpacks the the HolesConveyor struct for the
        candidate holes and the image
        from the vision_communications::CandidateHolesVectorMsg message
        @param[in] holesMsg
        [vision_communications::CandidateHolesVectorMsg&] The input
        candidate holes message
        @param[out] conveyor [HolesConveyor*] The output conveyor
        struct
        @param[out] image [cv::Mat*] The output image
        @param[in] encoding [const std::string&] The image's encoding
        @return void
       **/
      void unpackMessage(
        const vision_communications::CandidateHolesVectorMsg& holesMsg,
        HolesConveyor* conveyor, cv::Mat* image, const std::string& encoding);

      /**
        @brief Tests the merging operations on artificial holes
        @param[out] dummy [HolesConveyor*] The hole candidates
        @return void
       **/
      void testDummyHolesMerging(HolesConveyor* dummy);


    public:

      /**
        @brief The HoleFusion constructor
       **/
      HoleFusion(void);

      /**
        @brief The HoleFusion deconstructor
       **/
      ~HoleFusion(void);

  };

} // namespace pandora_vision

#endif  // HOLE_FUSION_NODE_HOLE_FUSION_H
