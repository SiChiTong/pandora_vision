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
__author__ = "Houtas Vasileios, Tsirigotis Christos, Kofinas Miltiadis"
__maintainer__ = "Kofinas Miltiadis"
__email__ = "mkofinas@gmail.com"

import os
import unittest

import rospy
import rosbag

import math
import numpy
import time
from collections import defaultdict
import cv2
from cv_bridge import CvBridge, CvBridgeError

from pandora_testing_tools.testing_interface import test_base
from sensor_msgs.msg import Image
from sensor_msgs.msg import PointCloud2

class VisionBenchmarkTestBase(test_base.TestBase):
    def readImages(self, imagePath):
        self.images = []
        self.names = []
        imageTypes = [".png", ".jpg", ".bmp", ".pgm"]
        bridge = CvBridge()
        if not os.path.isdir(imagePath):
            rospy.logerr("ERROR : Incorrect Path for image dataset")
            exit(1)

        for fileName in sorted(os.listdir(imagePath)):
            isImage = False
            for type in imageTypes:
                if (type in fileName):
                    isImage = True
                    break
            if (not isImage):
                rospy.logdebug("%s is not an image", fileName)
                continue
            # Read the next image.
            currentImg = cv2.imread(os.path.join(imagePath,
                                    fileName), -1)

            # If the image was not read succesfully continue to the
            # next file.
            if (currentImg is None):
                rospy.logdebug("Error reading image %s", fileName)
                rospy.logdebug("The process will read the next image!")
                continue

            # If the tested algorithm is not Victim, there is no need to use
            # depth and thermal images.
            if (("depth" in fileName or "thermal" in fileName) and
                (self.algorithm != "Victim" and self.algorithm != "Hole")):

                continue
            # Store the image.
            self.images.append(bridge.cv2_to_imgmsg(currentImg, "bgr8"))
            self.names.append(fileName)

    def readRosBags(self, imagePath):
        self.images = []
        self.names = []
        imageTypes = [".bag"]
        if not os.path.isdir(imagePath):
            rospy.logerr("ERROR : Incorrect Path for image dataset")
            exit(1)

        for fileName in sorted(os.listdir(imagePath)):
            isImage = False
            for type in imageTypes:
                if (type in fileName):
                    isImage = True
                    break
            if (not isImage):
                rospy.logdebug("%s is not a bag", fileName)
                continue
            # Read the next bag.
            currentBag = rosbag.Bag(os.path.join(imagePath, fileName), "r")
            # If the bag was not read succesfully continue to the
            # next file.
            if (currentBag is None):
                rospy.logdebug("Error reading bag %s", fileName)
                rospy.logdebug("The process will read the next bag!")
                continue
            # Store the bag.
            tempPointCloud = PointCloud2()
            for topic, msg, t in currentBag.read_messages("/camera/depth_registered/points"):
                tempPointCloud.header = msg.header
                tempPointCloud.height = msg.height
                tempPointCloud.width = msg.width
                tempPointCloud.fields = msg.fields
                tempPointCloud.is_bigendian = msg.is_bigendian
                tempPointCloud.point_step = msg.point_step
                tempPointCloud.row_step = msg.row_step
                tempPointCloud.data = msg.data
                tempPointCloud.is_dense = msg.is_dense

                self.images.append(tempPointCloud)

            self.names.append(fileName)
            currentBag.close()

    def readBenchmarkFile(self, filePath):
        filePath, pathTail = os.path.split(filePath)
        benchmarkFileName = filePath + "/vision_benchmarking.txt"
        if not os.path.isdir(filePath):
            rospy.logerr("ERROR : Incorrect Path for benchmark file.")
            exit(3)
        benchmarkFile = open(benchmarkFileName, "rU")
        self.benchmarkDict = defaultdict(list)
        for line in benchmarkFile:
            if line.startswith("#"):
                continue
            (frameName, objectType, distance, hAngle, vAngle,
                specs) = line.split(", ")
            if self.algorithm != "Victim" and "depth" in frameName:
                continue
            specs = specs.replace("\n", "")
            self.benchmarkDict[objectType, float(distance),
                float(hAngle), float(vAngle), specs].append(frameName)
        benchmarkFile.close()

    def readAnnotatorFile(self, filePath):
        filePath, pathTail = os.path.split(filePath)
        annotatorFileName = filePath + "/vision_annotations.txt"
        if not os.path.isdir(filePath):
            rospy.logerr("ERROR : Incorrect Path for annotator file.")
            exit(2)
        annotatorFile = open(annotatorFileName, "rU")
        self.annotatorDict = defaultdict(list)
        for line in annotatorFile:
            if line.startswith("#"):
                continue
            (frameName, objectType, xTopLeftPoint, yTopLeftPoint,
                xBottomRightPoint, yBottomRightPoint) = line.split(", ")
            if self.algorithm != "Victim" and "depth" in frameName:
                continue
            yBottomRightPoint = yBottomRightPoint.replace("\n", "")
            self.annotatorDict[frameName].append((objectType,
                                            int(xTopLeftPoint),
                                            int(yTopLeftPoint),
                                            int(xBottomRightPoint),
                                            int(yBottomRightPoint)))
        annotatorFile.close()

    def calculateAlertDistanceError(self, imageName, imageX, imageY):
        minSqrError = float("Inf")
        for annotatorKey, annotatorValues in self.annotatorDict.iteritems():
            if annotatorKey == imageName:
                minSqrError = float("Inf")
                for ii in xrange(len(annotatorValues)):
                    if self.algorithm == annotatorValues[ii][0]:
                        alertCenterX = ((annotatorValues[ii][1] +
                                        annotatorValues[ii][3]) / 2.0)
                        alertCenterY = ((annotatorValues[ii][2] +
                                        annotatorValues[ii][4]) / 2.0)
                        alertSqrDist = ((imageX - alertCenterX) ** 2 +
                                        (imageY - alertCenterY) ** 2)
                        if minSqrError > alertSqrDist:
                            minSqrError = alertSqrDist
                rospy.logdebug("Minimum Square Error: %f", minSqrError)
        return minSqrError

    def findAnnotations(self, imageName):
        annotations = 0
        for annotatorKey, annotatorValues in self.annotatorDict.iteritems():
            if annotatorKey == imageName:
                for ii in xrange(len(annotatorValues)):
                    if self.algorithm == annotatorValues[ii][0]:
                        annotations += 1
        return annotations

    def updateActualPositiveDictionary(self):
        for dictKey, dictValues in self.benchmarkDict.iteritems():
            if dictKey[0] == self.algorithm:
                distance = dictKey[1]
                hAngle = dictKey[2]
                vAngle = dictKey[3]
                specs = dictKey[4]
                num = 0
                for ii in xrange(len(dictValues)):
                    if dictValues[ii] in self.names:
                        num += 1
                if num > 0:
                    self.actualPositivesBenchmarkDict[
                        self.algorithm, float(distance), float(hAngle),
                        float(vAngle), specs] += num

    def updateTruePositiveDictionary(self, imageName):
        for dictKey, dictValues in self.benchmarkDict.iteritems():
            if (imageName in dictValues and
                    dictKey[0] == self.algorithm):
                distance = dictKey[1]
                hAngle = dictKey[2]
                vAngle = dictKey[3]
                specs = dictKey[4]
                self.truePositivesBenchmarkDict[self.algorithm,
                                                float(distance),
                                                float(hAngle),
                                                float(vAngle),
                                                specs] += 1

    def updateAdditionalInfoDict(self, imageName, alertsList):
        if self.algorithm == "LandoltC":
            numOfAngles = len(alertsList.angles)
            for dictKey, dictValues in self.benchmarkDict.iteritems():
                if (imageName in dictValues and
                        dictKey[0] == self.algorithm):
                    distance = dictKey[1]
                    hAngle = dictKey[2]
                    vAngle = dictKey[3]
                    specs = dictKey[4]
                    self.additionalInfoDict[self.algorithm,
                                            float(distance),
                                            float(hAngle),
                                            float(vAngle),
                                            specs] += numOfAngles

    def checkCorrectAlert(self, imageName, alertsList):
        if self.algorithm == "Hazmat":
            isCorrectAlert = False
            for dictKey, dictValues in self.benchmarkDict.iteritems():
                if (imageName in dictValues and
                    dictKey[0] == self.algorithm and
                    dictKey[4] == str(alertsList.patternType)):

                    isCorrectAlert = True
        else:
            isCorrectAlert = True
        return isCorrectAlert

    def calculateBenchmarkResults(self):
        rospy.loginfo("Benchmarking Test Results for %s node.", self.algorithm)
        rospy.loginfo("Number of images used in algorithms: %d",
                      len(self.images))
        rospy.loginfo("Number of True Positives: %d",
                      self.truePositives)
        rospy.loginfo("Number of False Positives: %d",
                      self.falsePositives)
        rospy.loginfo("Number of True Negatives: %d",
                      self.trueNegatives)
        rospy.loginfo("Number of False Negatives: %d",
                      self.falseNegatives)

        if len(self.elapsedTimeList) > 0:
            minElapsedTime = min(self.elapsedTimeList)
            maxElapsedTime = max(self.elapsedTimeList)
            meanElapsedTime = numpy.mean(self.elapsedTimeList)
            stdElapsedTime = numpy.std(self.elapsedTimeList)

            rospy.loginfo("Minimum Elapsed Time: %f seconds",
                          minElapsedTime)
            rospy.loginfo("Maximum Elapsed Time: %f seconds",
                          maxElapsedTime)
            rospy.loginfo("Mean Elapsed Time : %f seconds",
                          meanElapsedTime)
            rospy.loginfo("Standard Deviation of Elapsed Time: %f seconds",
                          stdElapsedTime)
        else:
            rospy.loginfo("No alerts found. Cannot display elapsed time.")

        accuracy = 0.0
        if (self.truePositives + self.falsePositives +
                self.trueNegatives + self.falseNegatives > 0):
            accuracy = (float(self.truePositives + self.trueNegatives) /
                        (self.truePositives + self.falsePositives +
                         self.trueNegatives + self.falseNegatives))

        precision = 0.0
        if self.truePositives + self.falsePositives > 0:
            precision = (float(self.truePositives) /
                         (self.truePositives + self.falsePositives))

        recall = 0.0
        if self.truePositives + self.falseNegatives > 0:
            recall = (float(self.truePositives) /
                      (self.truePositives + self.falseNegatives))

        fMeasure = 0.0
        if (self.truePositives + self.falseNegatives +
                self.falsePositives > 0):
            fMeasure = (2.0 * self.truePositives /
                        (2.0 * self.truePositives +
                        self.falseNegatives + self.falsePositives))

        if self.truePositives > 0:
            self.meanSquaredError /= float(self.truePositives)

        rospy.loginfo("Accuracy: %f", accuracy)
        rospy.loginfo("Precision: %f", precision)
        rospy.loginfo("Recall (TPR): %f", recall)
        rospy.loginfo("F-measure: %f", fMeasure)
        rospy.loginfo("Mean Squared Error from POI: %f", self.meanSquaredError)

    def calculateRecallResults(self):
        for dictKeyTP, dictValuesTP in self.actualPositivesBenchmarkDict.iteritems():
            if dictKeyTP in self.truePositivesBenchmarkDict:
                truePos = self.truePositivesBenchmarkDict[dictKeyTP]
            else:
                truePos = 0
            rospy.loginfo("%s: %d/%d", dictKeyTP, truePos, dictValuesTP)

    def calculateMeanNumberOfAngles(self):
        if self.algorithm == "LandoltC":
            rospy.loginfo("Mean Number of Landoltc found for each set of parameters")
            for dictKeyTP, dictValuesTP in self.truePositivesBenchmarkDict.iteritems():
                if dictKeyTP in self.additionalInfoDict:
                    numOfAngles = self.additionalInfoDict[dictKeyTP]
                    rospy.loginfo("%s: %f", dictKeyTP,
                                  float(numOfAngles) / dictValuesTP)

    def calculateAlertCenterPoint(self, yaw, pitch):
        imageX = (self.imageWidth * math.tan(yaw) /
                  (2.0 * math.tan(self.imageHFOV / 2.0)))
        imageY = (self.imageHeight * math.tan(pitch) /
                  (2.0 * math.tan(self.imageVFOV / 2.0)))
        imageX += self.imageWidth / 2.0
        imageY = -imageY + self.imageHeight / 2.0
        return imageX, imageY

    def benchmarkTest(self, imagePath, inputTopic, outputTopic):
        # Read a Set of Images
        rospy.loginfo("Reading Images")
        if self.algorithm == "Hole" or self.algorithm == "Victim":
            self.readRosBags(imagePath)
        else:
            self.readImages(imagePath)
        # Read Annotations for a Set of Images
        rospy.loginfo("Reading Annotator File")
        self.readAnnotatorFile(imagePath)
        # Read Benchmark Parameters for a Set of Images
        rospy.loginfo("Reading Benchmark File")
        self.readBenchmarkFile(imagePath)
        # Create Helper Structures for Benchmarking
        self.truePositivesBenchmarkDict = defaultdict(int)
        self.actualPositivesBenchmarkDict = defaultdict(int)
        self.updateActualPositiveDictionary()
        # Test Parameters and Variables
        self.truePositives = 0
        self.falsePositives = 0
        self.trueNegatives = 0
        self.falseNegatives = 0
        self.meanSquaredError = 0.0
        self.elapsedTimeList = []
        self.additionalInfoDict = defaultdict(int)
        errorThreshold = 3000.0
        maxWaitTime = 1.0
        bridge = CvBridge()
        if self.algorithm == "Hole":
            suffix = "sDirections"
        else:
            suffix = "Alerts"

        queueIndex = 0
        # Publish image files sequentially and wait for an alert.
        # Confirm the authenticity of the alert using the annotator
        # groundtruth set.
        for image, imageName in zip(self.images, self.names):
            rospy.logdebug("Sending Image %s", imageName)
            timeFlag = False
            startTime = time.clock()
            self.mockPublish(inputTopic, outputTopic, image)
            while not timeFlag:
                elapsedTime = time.clock() - startTime
                timeFlag = (elapsedTime > maxWaitTime or
                            (self.repliedList[outputTopic] and
                             len(self.messageList[outputTopic]) >= 1))

            if ((self.repliedList[outputTopic]) and
               (len(self.messageList[outputTopic]) >= 1)):
                rospy.logdebug("Alert found in Image %s", imageName)
                rospy.logdebug("Time passed: %f seconds", elapsedTime)
                truePositivesInImage = 0
                # Estimate alert center point from message parameters
                print self.messageList[outputTopic][queueIndex]
                alerts = getattr(
                        self.messageList[outputTopic][queueIndex],
                        self.algorithm.lower()+suffix)
                print len(alerts)
                queueIndex += 1
                for iiAlert in xrange(len(alerts)):
                    imageYaw = float(alerts[iiAlert].yaw)
                    imagePitch = float(alerts[iiAlert].pitch)
                    rospy.logdebug("Yaw: %f Degrees",
                                   imageYaw * 180 / math.pi)
                    rospy.logdebug("Pitch: %f Degrees",
                                   imagePitch * 180 / math.pi)
                    imageX, imageY = self.calculateAlertCenterPoint(
                            imageYaw, imagePitch)

                    minSqrError = self.calculateAlertDistanceError(
                            imageName, imageX, imageY)

                    isCorrectAlert = self.checkCorrectAlert(
                            imageName, alerts[iiAlert])

                    if minSqrError < errorThreshold and isCorrectAlert:
                        truePositivesInImage += 1
                        self.truePositives += 1
                        self.meanSquaredError += minSqrError
                        self.updateTruePositiveDictionary(imageName)
                        self.elapsedTimeList.append(elapsedTime)
                        self.updateAdditionalInfoDict(imageName,
                                alerts[iiAlert])
                    else:
                        self.falsePositives += 1

                annotationsInImage = self.findAnnotations(imageName)
                if annotationsInImage > truePositivesInImage:
                    self.falseNegatives += (annotationsInImage -
                                            truePositivesInImage)
            else:
                numberOfFalseNegatives = self.findAnnotations(imageName)
                if numberOfFalseNegatives > 0:
                    self.falseNegatives += numberOfFalseNegatives
                else:
                    self.trueNegatives += 1

        # Calculate the Benchmarking Results.
        self.calculateBenchmarkResults()
        # Estimate Recall values for each set of
        # (Distance, Horizontal Angle, Vertical Angle)
        self.calculateRecallResults()
        self.calculateMeanNumberOfAngles()
