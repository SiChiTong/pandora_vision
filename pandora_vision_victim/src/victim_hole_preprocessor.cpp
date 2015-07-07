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
 *   Tsirigotis Christos <tsirif@gmail.com>
 *   Chatzieleftheriou Eirini <eirini.ch0@gmail.com>
 *********************************************************************/

#include <string>

#include "pandora_vision_victim/victim_hole_preprocessor.h"

namespace pandora_vision
{
namespace pandora_vision_victim
{
  VictimHolePreProcessor::VictimHolePreProcessor()
    : sensor_processor::PreProcessor<pandora_vision_msgs::EnhancedImage,
    EnhancedImageStamped>()
  {}

  bool VictimHolePreProcessor::preProcess(const EnhancedImageConstPtr& input,
    const EnhancedImageStampedPtr& output)
  {
    output->setHeader(input->header);
    cv_bridge::CvImagePtr inMsg;

    inMsg = cv_bridge::toCvCopy(input->rgbImage, sensor_msgs::image_encodings::TYPE_8UC3);
    output->setRgbImage(inMsg->image.clone());

    inMsg = cv_bridge::toCvCopy(input->depthImage, sensor_msgs::image_encodings::TYPE_8UC1);
    cv::Mat depthImage = inMsg->image.clone();
    output->setDepthImage(depthImage);
    output->setDepth(input->isDepth);

    for (int ii = 0; ii < input->regionsOfInterest.size(); ii++)
    {
      Rect2f rect(input->regionsOfInterest[ii].center.x, input->regionsOfInterest[ii].center.y,
        input->regionsOfInterest[ii].width, input->regionsOfInterest[ii].height);
      output->setRegion(ii, rect);
    }
    return true;
  }
}  // namespace pandora_vision_victim
}  // namespace pandora_vision
