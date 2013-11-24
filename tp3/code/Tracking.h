#ifndef TRACKING_H
#define TRACKING_H

#include <vector>
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/nonfree/features2d.hpp"

#include "BackgroundExtractor.h"
/**
 * Tracking objects
 * using keypoints to detect objects
 * and KalmanFilter to tracking objects's velocity and position 
 */

class Tracking{
    public:
        /**
         * Constructor
         *
         */
        Tracking(char *file);
        void begin();
    private:
        char *videoFile;
        std::vector<cv::KalmanFilter> filters;
        cv::KalmanFilter filter;
        cv::Mat measurement = cv::Mat::zeros(2, 1, CV_32F);
        cv::Mat processNoise = cv::Mat(4, 1, CV_32F);
        cv::Mat state = cv::Mat(4, 1, CV_32F);
        cv::Mat background;
        cv::Mat foreground;
        cv::Mat tracker;
        
        bool initialised = false;
        cv::Mat frame;
        cv::Mat thresholdFrame;
        std::vector<cv::KeyPoint> keyPoints;
        std::vector<std::vector<cv::Point> > contours; 
        cv::VideoCapture capture;
        cv::BackgroundSubtractorMOG2 backgrounSubtructor;
        cv::SurfDescriptorExtractor extractor;
        std::vector<cv::Mat> descriptors;    
        cv::KeyPoint keyPoint;
        cv::Mat descriptor; //track this descriptor
        cv::BFMatcher matcher;
        cv::Mat video;
};
#endif

