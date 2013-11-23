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
#include <vector>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <math.h>
#include <thread>

#include "Detector.h"
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
void processHistory(std::vector<cv::Mat> history, cv::Mat &background){
    int rows = history.front().size().height;
    int cols = history.front().size().width;
    int numberOfHistoryFrame = history.size();
    //background = history[0].clone();
    cv::Mat b = cv::Mat::zeros(rows, cols, CV_8U);
    for(int row = 0; row < rows; row++ ){
        for(int col = 0; col < cols; col++){
            uchar pixels[numberOfHistoryFrame];
            for ( int k = 0; k < numberOfHistoryFrame; k++ ){
                pixels[k] = history[k].at<uchar>(row, col);
            }
            insertionSort(pixels, numberOfHistoryFrame);
            b.at<uchar>(row, col) = pixels[numberOfHistoryFrame/2 + 1];       
        }
    }
    background = b.clone();
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
//                    history.pop_front();
//                    history.push_back(frame);
                    std::thread t(processHistory, history, std::ref(background)); 
                    t.detach();
                    //process();
                    history.clear();
                    numberOfHistoryFrame = 0;
                }
            }
            //if(numberOfHistoryFrame > minNumberOfHistoryFrame && rnd == 0){
            //    process();
            //}
            foreground = subtract(gray, background);
            cv::erode(foreground,foreground,cv::Mat());
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

void Detector::process(std::vector<cv::Mat> hist, cv::Mat &bg){
    int rows = hist.front().size().height;
    int cols = hist.front().size().width;
    int numberOfHistoryFrame = hist.size();
    //background = history[0].clone();
    cv::Mat b = cv::Mat::zeros(rows, cols, CV_8U);
    for(int row = 0; row < rows; row++ ){
        for(int col = 0; col < cols; col++){
            uchar pixels[numberOfHistoryFrame];
            for ( int k = 0; k < numberOfHistoryFrame; k++ ){
                pixels[k] = hist[k].at<uchar>(row, col);
            }
            insertionSort(pixels, numberOfHistoryFrame);
            b.at<uchar>(row, col) = pixels[numberOfHistoryFrame/2 + 1];       
        }
    }
    bg = b.clone();
}

void Detector::process(){
    int rows = history.front().size().height;
    int cols = history.front().size().width;
    //background = history[0].clone();
    background = cv::Mat::zeros(rows, cols, CV_8U);
    for(int row = 0; row < rows; row++ ){
        for(int col = 0; col < cols; col++){
            uchar pixels[numberOfHistoryFrame];
            //int k = 0;
            //for ( std::list<cv::Mat>::iterator it = history.begin(); it != history.end(); ++it){
            for ( int k = 0; k < numberOfHistoryFrame; k++ ){
                //pixels[k] = cv::Mat(*it).at<uchar>(row, col);
                pixels[k] = history[k].at<uchar>(row, col);
                //k++;
            }
            insertionSort(pixels, numberOfHistoryFrame);
            background.at<uchar>(row, col) = pixels[numberOfHistoryFrame/2 + 1];    
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
