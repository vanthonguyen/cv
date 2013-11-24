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

#include "Tracking.h"

// plot points
#define drawCross(img, center, color, d )                               \
    line( img, cv::Point( center.x - d, center.y - d ),                 \
        cv::Point( center.x + d, center.y + d ), color, 1, CV_AA, 0);   \
    line( img, cv::Point( center.x + d, center.y - d ),                 \
        cv::Point( center.x - d, center.y + d ), color, 1, CV_AA, 0 )

#define debug(text)\
    std::cout<<text<<std::endl;

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

//    filter.statePre.at<float>(0) = 90*3;
//    filter.statePre.at<float>(1) = 284;
//    filter.statePre.at<float>(2) = 0;
//    filter.statePre.at<float>(3) = 0;
//    randn(filter.statePost, cv::Scalar::all(0), cv::Scalar::all(0.1));
    filter.statePost = *(cv::Mat_<float>(4, 1) << 270,284,0,0);
    measurement.at<float>(0) = 270;
    measurement.at<float>(1) = 283;
}

void Tracking::begin(){
    if(!capture.isOpened()){
        std::cerr << "Problem opening video source" << std::endl;
    }
    std::vector<cv::Vec4i> hierarchy;
    cv::HOGDescriptor hog;
    hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());

    cv::SimpleBlobDetector::Params params;                                                                                                                      
    params.minThreshold = 40;
    params.maxThreshold = 60;
    params.thresholdStep = 5;
    params.minArea = 600; 
    params.minConvexity = 0.3;
    params.minInertiaRatio = 0.01;
    params.maxArea = 8000;
    params.maxConvexity = 10;
    params.filterByColor = false;
    params.filterByCircularity = false;
   
    cv::SimpleBlobDetector blobDetector(params);
    blobDetector.create("SimpleBlob");
    std::vector<cv::KeyPoint> newKeyPoints;

    measurement.at<float>(0) = -1;
    measurement.at<float>(1) = -1;

    while((char)cv::waitKey(30) != 'q' && capture.grab()){
        capture.retrieve(frame);
        backgrounSubtructor.operator ()(frame, foreground);
        backgrounSubtructor.getBackgroundImage(background);
        cv::erode(foreground, foreground, cv::Mat());
        cv::dilate(foreground, foreground, cv::Mat());

        blobDetector.detect(foreground, newKeyPoints, cv::Mat());

        cv::findContours(foreground, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
        cv::drawContours(frame, contours, -1, cv::Scalar(0,0,255), 2);

        //if(measurement.at<float>(0) < 0){
        if(!initialised){
            tracker = cv::Mat::zeros(frame.size(), CV_8UC3);
        }
//        cv::drawKeypoints(foreground, newKeyPoints, tracker, CV_RGB(0,255,0), cv::DrawMatchesFlags::DEFAULT);
        if(newKeyPoints.size() > 0 && descriptors.size() == 0){
            for(int i = 0; i < newKeyPoints.size(); i++){
                cv::Mat des;
                std::vector<cv::KeyPoint> k(newKeyPoints.begin() + i, newKeyPoints.begin() + i + 1);
                extractor.compute(frame, k, des);
                descriptors.push_back(des);
            }
            //track first
            if(descriptor.size().width <= 0){
                descriptor = descriptors[0];
                keyPoint = newKeyPoints[0];
                cv::Point coordinate = keyPoint.pt;
                measurement.at<float>(0) = coordinate.x; 
                measurement.at<float>(1) = coordinate.y; 
            }
            
        }else if(newKeyPoints.size() > 0 && descriptors.size() > 0 ){
            std::vector<cv::Mat> newDescriptors;
            for(int i = 0; i < newKeyPoints.size(); i++){
                cv::Mat des;
                std::vector<cv::KeyPoint> k(newKeyPoints.begin() + i, newKeyPoints.begin() + i + 1);
                extractor.compute(frame, k, des);
                newDescriptors.push_back(des);
                for (int j = 0; j < descriptors.size(); j++){
                    cv::vector<cv::DMatch> matches;
                    matcher.match(des, descriptors[j], matches);
                }
                double bestMatch = 0.4;
                int index = -1;
                for(int k = 0; k < newDescriptors.size(); k++){
                    cv::vector<cv::DMatch> matches;
                    matcher.match(descriptor, newDescriptors[k], matches);
debug(matches[0].distance);
                    if(matches.size() > 0 && matches[0].distance < bestMatch ){
                        bestMatch = matches[0].distance;
                        index = k;
                    }
                }
                if( index >= 0){
                    descriptor = newDescriptors[index];
                    keyPoint = newKeyPoints[index];
                    measurement.at<float> (0) = keyPoint.pt.x; 
                    measurement.at<float> (1) = keyPoint.pt.y; 
                }
            }
        }
        if(measurement.at<float> (0) >= 0 ){
            initialised = true;
        }
        if(!initialised){
            cv::imshow("Video", frame);
            continue; 
        }
        cv::Mat prediction = filter.predict();
        cv::Point predictPt(prediction.at<float>(0), prediction.at<float>(1));
        drawCross( tracker, predictPt, cv::Scalar(255,0,0), 3 );
   
        cv::Point measurementPt(measurement.at<float>(0),measurement.at<float>(1));                                                                             
        //draw measurement 
        drawCross( tracker, measurementPt, cv::Scalar(0,255,0), 3 );

        cv::Mat estimated = filter.correct(measurement);
        cv::Point statePt(estimated.at<float>(0),estimated.at<float>(1)); 
        drawCross(frame, statePt, cv::Scalar(255, 255, 255), 5);
        //draw correction 
        drawCross( tracker, statePt, cv::Scalar(0,0,255), 3 );


        cv::imshow("Video", frame);
        cv::imshow("Tracker", tracker);
        //cv::imshow("Binary", thresholdFrame);
    }
}
