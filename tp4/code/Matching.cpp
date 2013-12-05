
#include "Matching.h"

#define drawCircle(img, center, r, color)\
        cv::circle(img, center, r, color)

#define drawLine(img, p1, p2, color)\
        cv::line(img, p1, p2, color)

int main(int argc, char ** argv){
    //devoir utiliser parametre
    if(argc < 2){
        printf("Parammeter is not enough, use ./matching image1 image2");
        return 2;
    }
    Matching matching(argv[1], argv[2]);
    matching.match();
    return 0;
}

Matching::Matching(char *img1, char *img2){
    image1 = cv::imread(img1, CV_LOAD_IMAGE_UNCHANGED);
    image2 = cv::imread(img2, CV_LOAD_IMAGE_UNCHANGED);
    combineImage();
}

void Matching::match(){
    getDescriptors();
    cv::vector<cv::DMatch> matches;
    cv::vector<cv::DMatch> goodMatches;
    matcher.match(descriptors1, descriptors2, matches);

    double maxDist = 0; 
    double minDist = 10000;

    //-- Quick calculation of max and min distances between keypoints
    for( int i = 0; i < descriptors1.rows; i++ ){
        double dist = matches[i].distance;
        if( dist < minDist ){
            minDist = dist;
        }
        if( dist > maxDist ){
            maxDist = dist;
        }
    }

    for( int i = 0; i < descriptors1.rows; i++ ){
        if( matches[i].distance <= 2*minDist ){
            int index1 = matches[i].queryIdx; 
            int index2 = matches[i].trainIdx; 
            cv::Point2f point1 = keyPoints1[index1].pt;
            cv::Point2f point2 = transformPoint(keyPoints2[index2].pt);
            cv::Scalar color(123,222,111);
            //draw circle
            cv::circle(image1, point1, 3, color);
            cv::line(image1, point1, point2, color, 2);
            goodMatches.push_back( matches[i]); 
        }
    }

    //-- Draw only "good" matches
    cv::Mat imgMatches;
    cv::drawMatches( image1, keyPoints1, image2, keyPoints2,
            goodMatches, imgMatches, cv::Scalar::all(-1), cv::Scalar::all(-1),
            std::vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );

    //-- Show detected matches
    cv::imshow( "Good Matches", imgMatches );
    cv::imshow( "Result", image1);
    
    
    while(cv::waitKey() != 27){

    };
}
void Matching::getDescriptors(){
    featureDetector.detect(image1, keyPoints1);  
    featureDetector.detect(image2, keyPoints2);  

    descriptorExtractor.compute(image1, keyPoints1, descriptors1);
    descriptorExtractor.compute(image2, keyPoints2, descriptors2);
}

cv::Point2f Matching::transformPoint(cv::Point2f p2){
    cv::Point2f re;
    re.x = p2.x * image1.cols/image2.cols;
    re.y = p2.y * image1.rows/image2.rows;
    return re;
}

void Matching::combineImage(){
    int cols = image1.cols + image2.cols;
    int rows = image1.rows > image2.rows ? image1.rows : image2.rows;
    output = cv::Mat::zeros(rows, cols, image2.type());
    //image1 is heigher than image2
    if(image1.rows >= rows){
        image1.copyTo(output(cv::Rect(0, 0, image1.cols, image1.rows)));
        image2.copyTo(output(cv::Rect(image1.cols, rows - image2.rows, image2.cols, image2.rows)));
    }else{
        image1.copyTo(output(cv::Rect(0, rows - image2.rows, image1.cols, image1.rows)));
        image2.copyTo(output(cv::Rect(cols - image1.cols, 0, image2.cols, image2.rows)));
    }
}
