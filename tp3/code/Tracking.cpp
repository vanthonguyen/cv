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
#define drawCircle(img, center, color, r)\
    cv::circle(img, center, r, color)
#define drawRectangle(img, center, color, d)\
    cv::rectangle(img, cv::Point(center.x -d , center.y -d), cv::Point(center.x +d , center.y +d), color)
#define debug(text)\
    std::cout<<text<<std::endl;

int main(int argc, char ** argv){
    Tracking tracking(argv[1]);        
    tracking.begin();
}
Tracking::Tracking(char *file): videoFile(file), capture(videoFile){
}

void Tracking::begin(){
    if(!capture.isOpened()){
        std::cerr << "Problem opening video source" << std::endl;
    }

    cv::SimpleBlobDetector::Params params;                                                                                                                      
    params.minThreshold = 40;
    params.maxThreshold = 60;
    params.thresholdStep = 5;
    params.minArea = 300; 
    params.minConvexity = 0.3;
    params.minInertiaRatio = 0.01;
    params.maxArea = 8000;
    params.maxConvexity = 10;
    params.filterByColor = false;
    params.filterByCircularity = false;
   
    cv::SimpleBlobDetector blobDetector(params);
    blobDetector.create("SimpleBlob");
    std::vector<cv::KeyPoint> newKeyPoints;

    while((char)cv::waitKey(30) != 'q' && capture.grab()){
        capture.retrieve(frame);
        backgrounSubtructor.operator ()(frame, foreground);
        backgrounSubtructor.getBackgroundImage(background);
        cv::erode(foreground, foreground, cv::Mat());
        cv::dilate(foreground, foreground, cv::Mat());
        cv::dilate(foreground, foreground, cv::Mat());
        cv::erode(foreground, foreground, cv::Mat());
        blobDetector.detect(foreground, newKeyPoints, cv::Mat());

        cv::findContours(foreground, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
        cv::drawContours(frame, contours, -1, cv::Scalar(0,0,255), 2);

        //if(measurement.at<float>(0) < 0){
        if(!initialised){
            tracker = cv::Mat::zeros(frame.size(), CV_8UC3);
            initialised = true;
        }
        //cv::drawKeypoints(foreground, newKeyPoints, tracker, CV_RGB(0,255,0), cv::DrawMatchesFlags::DEFAULT);
        std::vector<cv::Mat> newDescriptors;
        for(int i = 0; i < newKeyPoints.size(); i++){
            cv::Mat des;
            std::vector<cv::KeyPoint> k(newKeyPoints.begin() + i, newKeyPoints.begin() + i + 1);
            extractor.compute(frame, k, des);
            newDescriptors.push_back(des);
        }

        if(newDescriptors.size() > 0 && objects.size() == 0){
            for(int i = 0; i < newDescriptors.size(); i++){
                cv::Mat des;
                std::vector<cv::KeyPoint> k(newKeyPoints.begin() + i, newKeyPoints.begin() + i + 1);
                extractor.compute(frame, k, des);
                descriptors.push_back(des);
                MovingObject obj(newKeyPoints[i], newDescriptors[i]);
                objects.push_back(obj);
            }
        }else if(newDescriptors.size() > 0 && objects.size() > 0 ){
            for(int objIndex = 0; objIndex < objects.size(); objIndex++){
                double bestMatch = minDistance;
                int found = -1;
                for(int desIndex = 0; desIndex < newDescriptors.size(); desIndex++){
                    cv::vector<cv::DMatch> matches;
                    matcher.match(objects[objIndex].descriptor, newDescriptors[desIndex], matches);
                    if(matches.size() > 0 && matches[0].distance < bestMatch ){
                        bestMatch = matches[0].distance;
                        found = desIndex;
                    }
                }
                if( found >= 0){
                    objects[objIndex].descriptor = newDescriptors[found];
                    objects[objIndex].keyPoint = newKeyPoints[found];
                    cv::Point newPos = newKeyPoints[found].pt; 
                    objects[objIndex].process(newPos);
                    newKeyPoints.erase(newKeyPoints.begin() + found);
                    newDescriptors.erase(newDescriptors.begin() + found);

                    //draw new Point
                    drawCross(tracker, objects[objIndex].getMeasurement(), objects[objIndex].measurementColor, 2);
                    drawCircle(tracker, objects[objIndex].getPrediction(), objects[objIndex].predictionColor, 2);
                    drawRectangle(tracker, objects[objIndex].getEstimatedPosition(), objects[objIndex].estimatedColor, 2);
  //                  objects[objIndex].draw(tracker);
                    objects[objIndex].found++;
                }else{
                    objects[objIndex].notFoundCount++;
                }
                if(objects[objIndex].notFoundCount > 0 && objects[objIndex].found == 0){
                    objects.erase(objects.begin() + objIndex);
                }else if(objects[objIndex].notFoundCount > 10){
                    objects.erase(objects.begin() + objIndex);
                }
            }
                //add new objects
            for(int i = 0; i < newDescriptors.size(); i++){
                MovingObject obj(newKeyPoints[i], newDescriptors[i]);
                objects.push_back(obj);
            }
        }

debug(objects.size());
        cv::imshow("Video", frame);
        cv::imshow("Tracker", tracker);
//        cv::imshow("background", background);
    }
}
