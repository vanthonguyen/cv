#ifndef TRACKING_H
#define TRACKING_H

#include <vector>
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

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
        cv::KalmanFilter filter;
        cv::Mat measurement = cv::Mat::zeros(2, 1, CV_32F);
        cv::Mat processNoise = cv::Mat(4, 1, CV_32F);
        cv::Mat state = cv::Mat(4, 1, CV_32F);
        cv::Mat noise;
        cv::Mat frame;
        cv::Mat thresholdFrame;
        std::vector<cv::Mat> channels;
        std::vector<std::vector<cv::Point> > contours; 
        cv::VideoCapture capture;

        cv::Mat video;
};
#endif

