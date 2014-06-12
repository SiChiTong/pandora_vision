#!/usr/bin/env python
# Software License Agreement
__version__ = "0.0.1"
__status__ = "Production"
__license__ = "BSD"
__copyright__ = "Copyright (c) 2014, P.A.N.D.O.R.A. Team. All rights reserved."
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided
#    with the distribution.
#  * Neither the name of P.A.N.D.O.R.A. Team nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
__author__ = "Tsirigotis Christos"
__maintainer__ = "Tsirigotis Christos"
__email__ = "tsirif@gmail.com"

PKG = 'pandora_vision_hole_detector'
NAME = 'hole_detector_test'

import sys
import math

import unittest

import roslib; roslib.load_manifest(PKG)
import rostest
import rospy

from pandora_testing_tools.testing_interface import test_base
from vision_communications.msg import HolesDirectionsVectorMsg

class HoleDetectorTest(test_base.TestBase):

    def test_hole_detector(self):

       self.playFromBag(block = True)
       rospy.sleep(1.5)
       self.assertTrue(self.replied)
       #  how many alert messages have been published
       self.assertEqual(len(self.alertList), 1) 
       #  how many alerts have been sent with the first message
       self.assertEqual(len(self.alertList[0].holesDirections), 2)
       alert1 = self.alertList[0].holesDirections[0]
       alert2 = self.alertList[0].holesDirections[1]
       expectedX = 399 - 320
       expectedYaw = math.atan((2 * float(expectedX) / 640) * math.tan(58 * math.pi / 360))
       expectedY = 240 - 360
       expectedPitch = math.atan((2 * float(expectedY) / 480) * math.tan(45 * math.pi / 360))
       self.assertAlmostEqual(alert1.yaw, expectedYaw)
       self.assertAlmostEqual(alert1.pitch, expectedPitch)
       self.assertGreater(alert1.probability > 0.9)

if __name__ == '__main__':

    rospy.sleep(15)
    rospy.init_node(NAME, anonymous=True, log_level=rospy.DEBUG)
    subscriber_topics = [("/vision/holes_direction", "vision_communications", "HolesDirectionsVectorMsg")]
    publisher_topics = []
    HoleDetectorTest.connect(subscriber_topics, publisher_topics)
    rostest.rosrun(PKG, NAME, HoleDetectorTest, sys.argv)


