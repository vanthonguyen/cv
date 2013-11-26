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
    filter.processNoiseCov = *(cv::Mat_<float>(4,4) << 0.1,0,0.1,0,  0,0.1,0,0.1,  0,0,0.1,0,  0,0,0,0.1);
    cv::setIdentity(filter.measurementMatrix);
    cv::setIdentity(filter.processNoiseCov, cv::Scalar::all(1e-5));
    cv::setIdentity(filter.measurementNoiseCov, cv::Scalar::all(1e-1));
    cv::setIdentity(filter.errorCovPost, cv::Scalar::all(1));

    cv::Point initialPosition = keyPoint.pt; 
//    randn(filter.statePost, cv::Scalar::all(0), cv::Scalar::all(0.1));
    filter.statePost = *(cv::Mat_<float>(4, 1) << initialPosition.x,initialPosition.y,0,0);
    //int c = rng.uniform(100, 255);
    int r = rand()%256;
    int g = rand()%256;
    int b = rand()%256;
    predictionColor = cv::Scalar(b, g, r);
    measurementColor = cv::Scalar(b, g, r);
    estimatedColor= cv::Scalar(b, g, r);
//    measurement.at<float>(0) = initialPosition.x;
//    measurement.at<float>(1) = initialPosition.y;
}

void MovingObject::process(cv::Point measurementPosition){
    setPrediction(filter.predict());
    setMeasurement(measurementPosition);
    setEstimated(filter.correct(measurement));
}

void MovingObject::draw(cv::Mat &img){
    cv::line(img, prevMeasurement, getMeasurement(), measurementColor);
    cv::line(img, prevPrediction, getPrediction(), predictionColor);
    cv::line(img, prevEstimated, getEstimatedPosition(), estimatedColor);

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

void MovingObject::setEstimated(cv::Mat est){
    prevEstimated.x = estimated.at<float>(0);
    prevEstimated.y = estimated.at<float>(1);
    estimated = est;
}
void MovingObject::setPrediction(cv::Mat pred){
    prevPrediction.x = prediction.at<float>(0);
    prevPrediction.y = prediction.at<float>(1);
    prediction = pred;
}
void MovingObject::setMeasurement(cv::Point meas){
    prevMeasurement.x = measurement.at<float>(0);
    prevMeasurement.y = measurement.at<float>(1);
    measurement.at<float> (0) = meas.x;
    measurement.at<float> (1) = meas.y;
}
