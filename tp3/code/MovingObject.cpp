/*
 * =====================================================================================
 *
 *       Filename:  Tracking.cpp
 *
 *    Description:  Tracking objects in video using Kalman filter 
 *
 *        Version:  1.0
 *        Created:  11/23/2013 10:47:42 AM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  NGUYEN Van Tho (), thonv133@gmail.com
 *   Organization:  IFI 
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include "MovingObject.h"


#define debug(text)\
    std::cout<<text<<std::endl;

MovingObject::MovingObject(cv::KeyPoint initialKeyPoint, cv::Mat initialDescriptor): 
    keyPoint(initialKeyPoint), descriptor(initialDescriptor){
    filter.init(4, 2, 0);
    //filter.transitionMatrix = *(cv::Mat_<float>(2, 2) << 1, 1, 0, 1);
    filter.transitionMatrix = *(cv::Mat_<float>(4, 4) << 1,0,1,0,   0,1,0,1,  0,0,1,0,  0,0,0,1);
    filter.processNoiseCov = *(cv::Mat_<float>(4,4) << 0.2,0,0.2,0,  0,0.2,0,0.2,  0,0,0.3,0,  0,0,0,0.3);
    cv::setIdentity(filter.measurementMatrix);
    cv::setIdentity(filter.processNoiseCov, cv::Scalar::all(1e-5));
    cv::setIdentity(filter.measurementNoiseCov, cv::Scalar::all(1e-1));
    cv::setIdentity(filter.errorCovPost, cv::Scalar::all(1));

//    randn(filter.statePost, cv::Scalar::all(0), cv::Scalar::all(0.1));
    //filter.statePost = *(cv::Mat_<float>(4, 1) << 270,284,0,0);
    cv::Point initialPosition = keyPoint.pt; 
    //int c = rng.uniform(100, 255);
    int c = rand()%256 + 100;
    predictionColor = cv::Scalar(c, 0, 0);
    measurementColor = cv::Scalar(0, c, 0);
    estimatedColor= cv::Scalar(0, 0, c);
    measurement.at<float>(0) = initialPosition.x;
    measurement.at<float>(1) = initialPosition.y;
}

void MovingObject::process(cv::Point measurementPosition){
    prediction = filter.predict();

    measurement.at<float>(0) = measurementPosition.x;
    measurement.at<float>(1) = measurementPosition.y;

    estimated = filter.correct(measurement);
}

cv::Point MovingObject::getEstimatedPosition(){
    cv::Point estimatedPt(estimated.at<float>(0), estimated.at<float>(1));
    return estimatedPt; 
}

cv::Point MovingObject::getPrediction(){
    cv::Point predictPt(prediction.at<float>(0), prediction.at<float>(1));
    return predictPt; 
}
cv::Point MovingObject::getMeasurement(){
    cv::Point measurementPt(measurement.at<float>(0),measurement.at<float>(1)); 
    return measurementPt; 
}
