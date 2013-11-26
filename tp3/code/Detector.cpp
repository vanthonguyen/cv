/*
 * =====================================================================================
 *
 *       Filename:  Detector.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/12/2013 12:48:19 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  NGUYEN Van Tho (), thonv133@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */


#include <iostream>
#include <opencv2/opencv.hpp> 
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <stdlib.h>
#include <unistd.h>
#include <list>
#include <vector>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <math.h>
#include <thread>
#include <mutex>

#include "Detector.h"

std::mutex mt;

void insertionSort(uchar arr[], int length){
    int i, j ,tmp;                                                                     
    for (i = 1; i < length; i++)  {  
        j = i;  
        while (j > 0 && arr[j - 1] > arr[j])
        {
            tmp = arr[j];
            arr[j] = arr[j - 1];
            arr[j - 1] = tmp;
            j--;
        }
    }
}

//float *histogram;
void processHistory(std::list<cv::Mat> history, std::list<cv::Mat> &backgroundHistory, cv::Mat &background){
    std::lock_guard<std::mutex> guard(mt);
    int rows = history.front().size().height;
    int cols = history.front().size().width;
    int numberOfHistoryFrame = history.size();
    //background = history[0].clone();
    cv::Mat b = cv::Mat::zeros(rows, cols, CV_8U);
    for(int row = 0; row < rows; row++ ){
        for(int col = 0; col < cols; col++){
            uchar pixels[numberOfHistoryFrame];
            int k = 0;
            for ( std::list<cv::Mat>::iterator it = history.begin(); it != history.end(); ++it){
                pixels[k] = (*it).at<uchar>(row, col);
                k++;
            }
            insertionSort(pixels, numberOfHistoryFrame);
            b.at<uchar>(row, col) = pixels[numberOfHistoryFrame/2 + 1];       
        }
    }
    if(backgroundHistory.size() == maxNumberOfBackground){
        backgroundHistory.pop_front();
    }
    //only add the stable background
    if(numberOfHistoryFrame == maxNumberOfHistoryFrame){
        backgroundHistory.push_back(b);
    }
    //backgroundHistory.push_back(b);
    int numberOfBackground = backgroundHistory.size();
    if(numberOfBackground == 10){
        cv::Mat newBg = cv::Mat::zeros(rows, cols, CV_8U);
        for(int row = 0; row < rows; row++ ){
            for(int col = 0; col < cols; col++){
                uchar pixels[numberOfBackground];
                int k = 0;
                for ( std::list<cv::Mat>::const_iterator it = backgroundHistory.begin(), end = backgroundHistory.end(); it != end; ++it){
                    pixels[k] = (*it).at<uchar>(row, col);
                    k++;
                }
                insertionSort(pixels, numberOfBackground);
                newBg.at<uchar>(row, col) = pixels[numberOfBackground/2 + 1];       
            }
        }
        background = newBg.clone();
    }else{
        background = b.clone();
    }
}


int main(int argc, char ** argv){
    //devoir utiliser parametre
    Detector detector(argv[1]);
//    detector.showVideo();
    detector.getBackground();
    return 0;
}

Detector::Detector(char *fileName): videoFile(fileName), capture(videoFile){
}

void Detector::getBackground(){
    if( capture.isOpened()){
        cv::Mat frame;
        bool ok = capture.read(frame);
       // cv::cvtColor(frame, frame, CV_BGR2GRAY);
        cv::cvtColor(frame, background, CV_BGR2GRAY);
        while( ok ){
            //convert to grayscale
            cv::Mat gray;
            cv::cvtColor(frame, gray, CV_BGR2GRAY);
            int rnd = rand()%sampleRate;
            if(rnd == 0){
                if(numberOfHistoryFrame < maxNumberOfHistoryFrame ){
                    //to grayscale
                    //cv::Mat gray;
                    history.push_back(gray);
                    numberOfHistoryFrame++;
                }else{
                    history.pop_front();
                    history.push_back(gray);
                    //std::thread t(processHistory, history, std::ref(background)); 
                    //t.detach();
            //        history.clear();
            //       numberOfHistoryFrame = 0;
                }
            }
            if(numberOfHistoryFrame > minNumberOfHistoryFrame && rnd == 0){
                std::thread t(processHistory, history, std::ref(backgroundHistory), std::ref(background)); 
                t.detach();
            }
            foreground = subtract(gray, background);
            cv::erode(foreground,foreground,cv::Mat());
            cv::dilate(foreground,foreground,cv::Mat());
            cv::dilate(foreground,foreground,cv::Mat());

            cv::imshow("Origine",frame);
            cv::imshow("Foreground",foreground);
            //cv::moveWindow("Foreground", 400, 0);
            cv::imshow("Background",background);
            //cv::moveWindow("Foreground", 800, 0);
            if(cv::waitKey(30) > 0){
//                exit(0);
            }
            ok = capture.read(frame);
        }
    }
}


void Detector::getBackgroundCV(){
    cv::Mat back;
    cv::Mat fore;
    cv::BackgroundSubtractorMOG2 bg(5, 9, false);
    //bg.set("nmixtures", 3);
    //bg.set("bShadowDetection", false);
    std::vector<std::vector<cv::Point> > contours;
    if( capture.isOpened()){
        cv::Mat frame;
        bool ok = capture.read(frame);
        while( ok ){
            bg.operator ()(frame,fore);
            bg.getBackgroundImage(back);
            cv::erode(fore,fore,cv::Mat());
            cv::dilate(fore,fore,cv::Mat());
            cv::findContours(fore,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
            cv::drawContours(frame,contours,-1,cv::Scalar(0,0,255),2);
            cv::imshow("xxx", frame);
            cv::imshow("Background",back);
            //cv::waitKey(1000/fps);
            cv::waitKey(33);
            ok = capture.read(frame);
        }
    }

}
void Detector::showVideo(){
    if( capture.isOpened()){
        cv::Mat frame;
        bool ok = capture.read(frame);
        while( ok ){
            cv::imshow("xxx", frame);
            //cv::waitKey(1000/fps);
            cv::waitKey(33);
            ok = capture.read(frame);
        }
    }
}

cv::Mat Detector::subtract(cv::Mat m1, cv::Mat m2){
    //check size, channels
    int rows = m1.size().height;
    int cols = m2.size().width;
    cv::Mat result = cv::Mat::zeros(m1.size(), CV_8U);
    for(int row = 0; row < rows; row++ ){
        for(int col = 0; col < cols; col++){
            uchar val = abs(m1.at<uchar> (row, col) - m2.at<uchar> (row, col));
            result.at<uchar> (row, col) = (val < threshold) ? 0 : val;
        }
    }
    return result;
}
void Detector::insertionSort(uchar arr[], int length){
    int i, j ,tmp;
    for (i = 1; i < length; i++)  {  
        j = i;  
        while (j > 0 && arr[j - 1] > arr[j])
        {
            tmp = arr[j];
            arr[j] = arr[j - 1];
            arr[j - 1] = tmp;
            j--;
        }
    }
}

void Detector::printr(uchar arr[], int length){
    for (int i = 0; i < length; i++){
        std::cout <<" "<< (int)arr[i];
    }
    std::cout << std::endl;
}
