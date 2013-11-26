#ifndef MOVING_OBJECT_H
#define MOVING_OBJECT_H

#include <vector>
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/nonfree/features2d.hpp"

/**
 * Tracking objects
 * using keypoints to detect objects
 * and KalmanFilter to tracking objects's velocity and position 
 */

class MovingObject{
    public:
        /**
         * Constructor
         *
         */
        MovingObject(cv::KeyPoint initialKeyPoint, cv::Mat initialDescriptor);
        void process(cv::Point measurementPosition);

        void draw(cv::Mat &mat);
        cv::Point getEstimatedPosition();
        cv::Point getMeasurement();
        cv::Point getPrediction();

        void setPrediction(cv::Mat pred);
        void setMeasurement(cv::Point meas);
        void setEstimated(cv::Mat est);

        //public variable difination
        cv::Scalar predictionColor;        //color for display the predict point
        cv::Scalar measurementColor;    //color for display the measurement point
        cv::Scalar estimatedColor;          //color for display the state point
        cv::KeyPoint keyPoint;
        cv::Mat descriptor; //track this descriptor
        int notFoundCount = 0;
        int found = 0;

//        cv::RNG rng;
    private:
        cv::KalmanFilter filter;
        cv::Mat measurement = cv::Mat::zeros(2, 1, CV_32F);
        cv::Mat estimated = cv::Mat(4, 1, CV_32F);
        cv::Mat prediction = cv::Mat(4, 1, CV_32F);
        cv::Point prevMeasurement;
        cv::Point prevEstimated;
        cv::Point prevPrediction;
};
#endif

