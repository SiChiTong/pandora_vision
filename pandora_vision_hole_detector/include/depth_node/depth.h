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
 * Authors: Alexandros Filotheou, Manos Tsardoulias
 *********************************************************************/

#ifndef DEPTH_NODE_DEPTH_H
#define DEPTH_NODE_DEPTH_H

#include "depth_node/hole_detector.h"
#include "utils/parameters.h"
#include "utils/message_conversions.h"
#include "vision_communications/DepthCandidateHolesVectorMsg.h"


/**
  @namespace vision
  @brief The main namespace for PANDORA vision
 **/
namespace pandora_vision
{
  class PandoraKinect
  {
    private:
      //!< The ROS node handle
      ros::NodeHandle     _nodeHandle;

      //!< The RGB image acquired from Kinect
      cv::Mat             _kinectFrame;

      //!< The Depth image acquired from Kinect
      cv::Mat             _kinectDepthFrame;

      //!< Timestamp of RGB image
      ros::Time           _kinectFrameTimestamp;

      //!< Timestamp of depth image
      ros::Time           _kinectDepthFrameTimestamp;

      //!< Subscriber of Kinect point cloud
      ros::Subscriber     _inputCloudSubscriber;

      //!< ROS publisher for computed 2D planes
      ros::Publisher      _planePublisher;

      //!< ROS publisher for the candidate holes
      ros::Publisher      _candidateHolesPublisher;

      //!< The dynamic reconfigure (depth) parameters' server
      dynamic_reconfigure::Server<pandora_vision_hole_detector::depth_cfgConfig>
        server;

      //!< The dynamic reconfigure (depth) parameters' callback
      dynamic_reconfigure::Server<pandora_vision_hole_detector::depth_cfgConfig>
        ::CallbackType f;


      /**
        @brief Callback for the point cloud
        @param msg [const sensor_msgs::PointCloud2ConstPtr&] The point cloud
        message
        @return void
       **/
      void inputCloudCallback
        (const sensor_msgs::PointCloud2ConstPtr& msg);


      /**
        @brief Extracts a CV_32FC1 depth image from a PointCloudXYZPtr
        point cloud
        @param pointCloudXYZ [const PointCloudXYZPtr&] The point cloud
        @param depthImage [cv::Mat*] The extracted depth image
        @return [cv::Mat] The depth image
       **/
      void extractDepthImageFromPointCloud(
        const PointCloudXYZPtr& pointCloudXYZPtr, cv::Mat* depthImage);

      /**
        @brief Stores a ensemble of point clouds in pcd images
        @param in_cloud [const std::vector<PointCloudXYZPtr>&] The point clouds
        @return void
       **/
      void storePointCloudVectorToImages
        (const std::vector<PointCloudXYZPtr>& in_cloud);

      /**
        @brief Constructs a vision_communications/DepthCandidateHolesVectorMsg
        message
        @param[in] conveyor [HolesConveyor&] A struct containing
        vectors of the holes' keypoints, bounding rectangles' vertices
        and blobs' outlines
        @param[in] interpolatedDepthImage [cv::Mat&] The denoised depth image
        @param[in] pointCloudXYZPtr [PointCloudXYZPtr&] The undistorted point
        cloud
        @param[out] depthCandidateHolesMsg
        [vision_communications::DepthCandidateHolesVectorMsg*] The output
        message
        @param[in] encoding [std::string&] The interpoladedDepth image's
        encoding
        @return void
       **/
      void createCandidateHolesMessage(
        const HolesConveyor& conveyor,
        const cv::Mat& interpolatedDepthImage,
        const PointCloudXYZPtr& pointCloudXYZPtr,
        vision_communications::DepthCandidateHolesVectorMsg*
        depthCandidateHolesMsg,
        const std::string& encoding);

      /**
        @brief Publishes the planes to /vision/kinect/planes topic
        @param cloudVector [const std::vector<PointCloudXYZPtr>&] The point
        clouds containing the planes
        @return void
       **/
      void publishPlanes
        (const std::vector<PointCloudXYZPtr>& cloudVector);

      /**
        @brief The function called when a parameter is changed
        @param[in] config [const pandora_vision_hole_detector::depth_cfgConfig&]
        @param[in] level [const uint32_t] The level (?)
        @return void
       **/
      void parametersCallback(
        const pandora_vision_hole_detector::depth_cfgConfig& config,
        const uint32_t& level);


    public:

      /**
        @brief Default constructor. Initiates communications, loads parameters.
        @return void
       **/
      PandoraKinect(void);

      /**
        @brief Default destructor
        @return void
       **/
      ~PandoraKinect(void);

  };

} // namespace pandora_vision

#endif  // DEPTH_NODE_DEPTH_H
