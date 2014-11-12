#include "pandora_vision_hazmat/hazmat_detector.h"
#include "pandora_vision_hazmat/histogram_mask.h"
#include "pandora_vision_hazmat/image_signature.h"
#include "pandora_vision_hazmat/sift_hazmat_detector.h"

int main(int argc , char **argv )
{
  SiftHazmatDetector detectorObj ;
  //~ Detector *detector = new HistogramMask(detectorObj);
  //~ ImageSignature sign(&detectorObj);
  //~ HistogramMask detector(&sign);
  HistogramMask detector(&detectorObj);
  
  cv::VideoCapture camera(1);
  
  if ( !camera.isOpened() )
  {
    std::cerr << "Error Opening the camera ! " << std::endl;
    return -1;
  }
  
  cv::Mat pattern = cv::imread("/home/vchoutas/Programming/pandora_ws/src/pandora_vision/pandora_vision_hazmat/src/test2.png");
  
  if ( !pattern.data )
  {
    std::cerr << "Could not open the image !" << std::endl;
    return -1;
  }
  
  
  
  cv::Mat frame;
  cv::Mat hist;
  cv::Mat mask;
  cv::Mat maskedFrame;
  HistogramMask::calcNormYUVHist(pattern , &hist);
  float x , y;
  
  while(true)
  {
    const clock_t begin_time = clock();
    
    camera.grab();
    camera.retrieve(frame);
    //~ bool found = detectorObj->detect(frame , &x, &y);
    //~ if (found)
    //~ {
      //~ cv::circle( frame , cv::Point2f(x,y)  , 4.0 , cv::Scalar(0,0,255) , -1 , 8 );
      //~ 
    //~ }
    if ( !frame.data )
    {
      std::cout << "Invalid Frame. Continuing to next iteration!" 
        << std::endl;
      continue;
    }
    
    //~ HistogramMask::normalizeImage(frame , &frame , 0);
    //~ cv::cvtColor(frame,saliency,CV_BGR2Lab);
    //~ cv::cvtColor(frame,saliency,CV_BGR2GRAY);

    //~ detector.createMask( frame , &mask);
    detector.createMask( frame , &mask , hist );
    
    if ( ! mask.data )
    {
      std::cerr << "Invalid Mask " << std::endl;
      // Release the camera .
      camera.release();
      break;
    }
    
    std::cout << ( clock () - begin_time ) /  
      static_cast<double>(CLOCKS_PER_SEC )<< std::endl;
    
    
    
    
    cv::imshow("Frame",frame);
    cv::imshow("Mask" , mask);
    frame.copyTo(maskedFrame,mask);
    cv::imshow("Segmented Frame",maskedFrame);

    if (cv::waitKey(30)>=0)
    { 
      // Release the camera.
      camera.release();
      break;
    }
    
    // Reset the contents of the mask and the Segmented Frame.
    mask.setTo(0);
    maskedFrame.setTo(0);
  }

  
  
  }
