/*
 * =====================================================================================
 *       Filename:  Matching.h
 *    Description: Matching 2 images using SIFT feature detector 
 *        Created:  12/05/2013 11:00:00 AM
 *       Compiler:  g++
 *         Author:  NGUYEN Van Tho (), thonv133@gmail.com
 *   Organization:  IFI 
 *
 * =====================================================================================
 */


#ifndef MATCHING_H
#define MATCHING_H

#include <vector>

#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/nonfree/features2d.hpp"

/* *
 * Using SIFT descriptor for matching 2 images
 * */
class Matching{
    public:
        /* *
         * Input: 2 images
         * */
        Matching(char *img1, char *img2);
        void match();
    private:
        //private methods
        void getDescriptors();

        //combine 2 images vertically
        void combineImage();

        cv::Point2f transformPoint(cv::Point2f p2);
        //private variables
        cv::Mat image1;
        cv::Mat image2;

        //output image
        cv::Mat output;

        //keypoints of image 1
        std::vector<cv::KeyPoint> keyPoints1;
        //keypoints of image 2
        std::vector<cv::KeyPoint> keyPoints2;
        //image of descriptors of image1
        cv::Mat descriptors1;
        //image of descriptors of image2
        cv::Mat descriptors2;

        //Sift feature detector and extractor
        cv::SiftDescriptorExtractor descriptorExtractor;
        cv::SiftFeatureDetector     featureDetector;
        
        //matcher
        cv::FlannBasedMatcher matcher;
};

#endif

