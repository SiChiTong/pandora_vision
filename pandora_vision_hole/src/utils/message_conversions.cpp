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

#include "utils/message_conversions.h"

/**
  @namespace pandora_vision
  @brief The main namespace for PANDORA vision
 **/
namespace pandora_vision
{
  /**
    @brief Converts a cv::Mat image into a sensor_msgs::Image message
    @param[in] image [const cv::Mat&] The image
    @param[in] encoding [const std::string&] The image message's encoding
    @param[in] msg [const sensor_msgs::Image&] A message needed for
    setting the output message's header by extracting its header
    @return [sensor_msgs::Image] The output image message
   **/
  sensor_msgs::Image
  MessageConversions::
  convertImageToMessage(
      const cv::Mat& image, const std::string& encoding,
      const std_msgs::Header& header)
  {
    cv_bridge::CvImagePtr msgPtr(new cv_bridge::CvImage());

    msgPtr->header = header;
    msgPtr->encoding = encoding;
    msgPtr->image = image;

    return *msgPtr->toImageMsg();
  }

  /**
    @brief Extracts an image from a point cloud message
    @param pointCloud[in] [const PointCloudPtr&]
    The input point cloud message
    @param[in] id [const int&] The enconding of the converted image.
    CV_32FC1 for a depth image, CV_8UC3 for a rgb image
    @return cv::Mat The output image
   **/
  cv::Mat
  MessageConversions::
  convertPointCloudMessageToImage(
      const PointCloudPtr& pointCloud, const int& encoding)
  {
    #ifdef DEBUG_TIME
    Timer::start("convertPointCloudMessageToImage");
    #endif

    // Prepare the output image
    cv::Mat image(pointCloud->height, pointCloud->width, encoding);

    // For the depth image
    if (encoding == CV_32FC1)
    {
      for (unsigned int row = 0; row < pointCloud->height; ++row)
      {
        for (unsigned int col = 0; col < pointCloud->width; ++col)
        {
          image.at<float>(row, col) =
            pointCloud->points[col + pointCloud->width * row].z;

          // if element is nan make it a zero
          if (image.at<float>(row, col) != image.at<float>(row, col))
          {
            image.at<float>(row, col) = 0.0;
          }
        }
      }
    }
    else if (encoding == CV_8UC3) // For the rgb image
    {
      for (unsigned int row = 0; row < pointCloud->height; ++row)
      {
        for (unsigned int col = 0; col < pointCloud->width; ++col)
        {
          image.at<unsigned char>(row, 3 * col + 2) =
            pointCloud->points[col + pointCloud->width * row].r;
          image.at<unsigned char>(row, 3 * col + 1) =
            pointCloud->points[col + pointCloud->width * row].g;
          image.at<unsigned char>(row, 3 * col + 0) =
            pointCloud->points[col + pointCloud->width * row].b;
        }
      }
    }

    #ifdef DEBUG_TIME
    Timer::tick("convertPointCloudMessageToImage");
    #endif

    return image;
  }

  /**
    @brief Extracts a cv::Mat image from a ROS image message
    @param[in] msg [const sensor_msgs::Image&] The input ROS image
    message
    @param[out] image [cv::Mat*] The output image
    @param[in] encoding [const std::string&] The image encoding
    @return void
   **/
  void
  MessageConversions::
  extractImageFromMessage(
      const sensor_msgs::Image& msg,
      cv::Mat* image,
      const std::string& encoding)
  {
    #ifdef DEBUG_TIME
    Timer::start("extractImageFromMessage");
    #endif

    cv_bridge::CvImagePtr in_msg;

    in_msg = cv_bridge::toCvCopy(msg, encoding);

    *image = in_msg->image.clone();

    #ifdef DEBUG_TIME
    Timer::tick("extractImageFromMessage");
    #endif
  }

  pandora_vision_msgs::Keypoint
  MessageConversions::
  cvToMsg(const cv::Point2f& point)
  {
    pandora_vision_msgs::Keypoint msg_point;
    msg_point.x = point.x;
    msg_point.y = point.y;
    return msg_point;
  }

  cv::Point2f
  MessageConversions::
  msgToCv(const pandora_vision_msgs::Keypoint& point)
  {
    cv::Point2f cv_point(point.x, point.y);
    return cv_point;
  }

  pandora_vision_msgs::Keypoint
  MessageConversions::
  keypointToMsg(const cv::KeyPoint& point)
  {
    pandora_vision_msgs::Keypoint msg_point;
    msg_point.x = point.pt.x;
    msg_point.y = point.pt.y;
    return msg_point;
  }

  cv::KeyPoint
  MessageConversions::
  msgToKeypoint(const pandora_vision_msgs::Keypoint& point)
  {
    cv::KeyPoint keypoint;
    keypoint.pt.x = point.x;
    keypoint.pt.y = point.y;
    return keypoint;
  }

  std::vector<pandora_vision_msgs::Keypoint>
  MessageConversions::
  vecToMsg(const std::vector<cv::Point2f>& vec)
  {
    std::vector<pandora_vision_msgs::Keypoint> temp;
    
    for (int ii = 0; ii < vec.size(); ii++)
    {
      temp.push_back(cvToMsg(vec[ii]));
    }
    return temp;
  }
  
  pandora_vision_msgs::AreaOfInterest
  MessageConversions::
  vecToArea(const std::vector<cv::Point2f>& vert)
  {
    pandora_vision_msgs::AreaOfInterest area;

    int minx = vert[0].x, maxx = vert[0].x, miny = vert[0].y, maxy = vert[0].y;
    for (unsigned int j = 1 ; j < 4 ; j++)
    {
      int xx = vert[j].x;
      int yy = vert[j].y;
      minx = xx < minx ? xx : minx;
      maxx = xx > maxx ? xx : maxx;
      miny = yy < miny ? yy : miny;
      maxy = yy > maxy ? yy : maxy;
    }
    area.width = maxx - minx;
    area.height = maxy - miny;
    area.center.x = minx + area.width / 2;
    area.center.y = miny + area.height / 2;

    return area;
  }
} // namespace pandora_vision
