
#ifndef DETECTOR_H 
#define DETECTOR_H 
 
#include <list>
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

class Detector 
{
    public:
        Detector(char * fileName);
        void showVideo();
        void getBackground();
        void getBackgroundCV();
//        virtual ~Detector();
	private:
        char * videoFile;
        int numberOfHistoryFrame = 0;
        const int maxNumberOfHistoryFrame = 20;
        const int minNumberOfHistoryFrame = 2;
        const int threshold = 20;
        cv::VideoCapture capture;
        cv::Mat background;
        cv::Mat foreground;
        std::list<cv::Mat> history;
        cv::Mat subtract(cv::Mat m1, cv::Mat m2);
        void process();
        void insertionSort(uchar arr[], int length);
        void printr(uchar arr[], int length);
};
#endif

