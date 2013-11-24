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

#include <vector>
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "Tracking.h"

// plot points
#define drawCross(img, center, color, d )                               \
    line( img, cv::Point( center.x - d, center.y - d ),                 \
        cv::Point( center.x + d, center.y + d ), color, 1, CV_AA, 0);   \
    line( img, cv::Point( center.x + d, center.y - d ),                 \
        cv::Point( center.x - d, center.y + d ), color, 1, CV_AA, 0 )


int main(int argc, char ** argv){
    Tracking tracking(argv[1]);        
    tracking.begin();
}
Tracking::Tracking(char *file): videoFile(file), capture(videoFile){
    filter.init(4, 2, 0);
    //filter.transitionMatrix = *(cv::Mat_<float>(2, 2) << 1, 1, 0, 1);
    filter.transitionMatrix = *(cv::Mat_<float>(4, 4) << 1,0,1,0,   0,1,0,1,  0,0,1,0,  0,0,0,1);
    filter.processNoiseCov = *(cv::Mat_<float>(4,4) << 0.2,0,0.2,0,  0,0.2,0,0.2,  0,0,0.3,0,  0,0,0,0.3);
    cv::setIdentity(filter.measurementMatrix);
    cv::setIdentity(filter.processNoiseCov, cv::Scalar::all(1e-5));
    cv::setIdentity(filter.measurementNoiseCov, cv::Scalar::all(1e-1));
    cv::setIdentity(filter.errorCovPost, cv::Scalar::all(1));

    filter.statePre.at<float>(0) = 0;
    filter.statePre.at<float>(1) = 0;
    filter.statePre.at<float>(2) = 0;
    filter.statePre.at<float>(3) = 0;
//    randn(filter.statePost, cv::Scalar::all(0), cv::Scalar::all(0.1));
}

void Tracking::begin(){
    if(!capture.isOpened()){
        std::cerr << "Problem opening video source" << std::endl;
    }
    std::vector<cv::Vec4i> hierarchy;
    while((char)cv::waitKey(30) != 'q' && capture.grab()){
        capture.retrieve(frame);
        cv::Mat gray;
        cv::cvtColor(frame, gray, CV_BGR2GRAY); 
        if(!initialised){
            tracker = cv::Mat::zeros(frame.size(), CV_8UC3); 
            initialised = true;
        }
        int rnd = rand()%sampleRate;
        if(rnd == 0){
            background = backgroundExtractor.getBackground(gray);
        }
        cv::split(frame, channels);
        //cv::subtract(channels[2], channels[1], channels[2]); 
        thresholdFrame = backgroundExtractor.subtract(gray, background );
        cv::threshold(thresholdFrame, thresholdFrame, 50, 255, CV_THRESH_BINARY);
        cv::erode(thresholdFrame,thresholdFrame,cv::Mat());                
        cv::dilate(thresholdFrame,thresholdFrame,cv::Mat());
        cv::medianBlur(thresholdFrame, thresholdFrame, 5);
        cv::findContours(thresholdFrame, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
        cv::Mat drawing = cv::Mat::zeros(thresholdFrame.size(), CV_8UC1);

        for(size_t i = 0; i < contours.size(); i++){
            if(cv::contourArea(contours[i]) > 100){
                std::cout<<cv::contourArea(contours[i])<<std::endl;
                cv::drawContours(drawing, contours, i, cv::Scalar::all(255), CV_FILLED, 8, std::vector<cv::Vec4i>(), 0, cv::Point());
            }else{
                contours.erase(contours.begin() + i);
            }
        }
        thresholdFrame = drawing;
        drawing = cv::Mat::zeros(thresholdFrame.size(), CV_8UC1);

        for(size_t i = 0; i < contours.size(); i++){
            if(cv::contourArea(contours[i]) > 100){
                std::cout<<cv::contourArea(contours[i])<<std::endl;
                cv::drawContours(drawing, contours, i, cv::Scalar::all(255), CV_FILLED, 8, std::vector<cv::Vec4i>(), 0, cv::Point());
            }else{
                contours.erase(contours.begin() + i);
            }
        }
        thresholdFrame = drawing;

        /* fix me: get moment here */
        std::vector<cv::Moments> mu(contours.size() );
        
        for( size_t i = 0; i < contours.size(); i++ ){
            mu[i] = cv::moments( contours[i], false ); 
        }

        //  Get the mass centers:
        std::vector<cv::Point2f> mc( contours.size() );
        for( size_t i = 0; i < contours.size(); i++ ){ 
            mc[i] = cv::Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 ); 
        }

        cv::Mat prediction = filter.predict();
        cv::Point predictPt(prediction.at<float>(0), prediction.at<float>(1));

        //draw prediction
        drawCross( tracker, predictPt, cv::Scalar(255,0,0), 3 );
        for(size_t i = 0; i < mc.size(); i++){
            drawCross(frame, mc[i], cv::Scalar(255, 0, 0), 5);
            measurement.at<float>(0) = mc[i].x;
            measurement.at<float>(1) = mc[i].y;
        }


        cv::Point measurementPt(measurement.at<float>(0),measurement.at<float>(1));

        //draw measurement 
        drawCross( tracker, measurementPt, cv::Scalar(0,255,0), 3 );

        cv::Mat estimated = filter.correct(measurement);
        cv::Point statePt(estimated.at<float>(0),estimated.at<float>(1));

        drawCross(frame, statePt, cv::Scalar(255, 255, 255), 5);
        //draw correction 
        drawCross( tracker, statePt, cv::Scalar(0,0,255), 3 );

        std::vector<std::vector<cv::Point>> contoursPoly( contours.size() );
        std::vector<cv::Rect> boundRect( contours.size() );
        for( size_t i = 0; i < contours.size(); i++ ){ 
            cv::approxPolyDP( cv::Mat(contours[i]), contoursPoly[i], 3, true );
            boundRect[i] = cv::boundingRect( cv::Mat(contoursPoly[i]) );
        }

        for( size_t i = 0; i < contours.size(); i++ ){
            cv::rectangle( frame, boundRect[i].tl(), boundRect[i].br(), cv::Scalar(0, 255, 0), 2, 8, 0 );
        }

        cv::imshow("Video", frame);
        cv::imshow("Tracker", tracker);
        cv::imshow("Binary", thresholdFrame);
    }
}
