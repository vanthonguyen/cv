#ifndef TRACKING_H
#define TRACKING_H


#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

class Tracking{
    public:
        /**
         * Constructor
         *
         */
        Tracking();
    private:
        cv::Mat state;
        cv::Mat noise;
        cv::Mat mesure;
};
#endif

