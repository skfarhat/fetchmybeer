//
//  test-color.cpp
//  FetchMyBeer
//
//  Created by Sami Farhat on 12/06/2016.
//
//

#include "test-color.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <map>

using namespace cv;
using std::cout;
using std::cerr;
using std::cin;
using std::endl;
using std::string;
using std::vector;
using std::map;
using std::multimap;

#define CTRL_HLOW         "hlow"
#define CTRL_HHIGH        "hhigh"
#define CTRL_VLOW         "vlow"
#define CTRL_VHIGH        "vhigh"
#define CTRL_SLOW         "slow"
#define CTRL_SHIGH        "shigh"
#define CTRL_C            "C"
#define CTRL_BLOCKSIZE    "BlockSize"

// =============================================================================
// TODO:

// *    Only show those rectangles and circles that are larger than some heuristic
//      area variable

// =============================================================================
class Trackbar {

public:
    int min;
    int max;
    int value;
    Trackbar(int _min, int _max, int _value)
    {
        min = _min;
        max = _max;
        value = _value;
    }
};

map<string, Trackbar*> controls;

const char* WINDOW_CONTROLS =  "display";

void trackbarChanged(int value, void * data)
{
}

void createTrackbars()
{
    
    controls[CTRL_HLOW] = new Trackbar(0,180,0);
    controls[CTRL_HHIGH] = new Trackbar(0,180,255);
    controls[CTRL_SLOW] = new Trackbar(0,255,0);
    controls[CTRL_SHIGH] = new Trackbar(0,255,255);
    controls[CTRL_VLOW] = new Trackbar(0,255,0);
    controls[CTRL_VHIGH] = new Trackbar(0,255,255);
    controls[CTRL_C] = new Trackbar(1,255, 3);
    controls[CTRL_BLOCKSIZE]= new Trackbar(1,255,7);
    
    
    for (auto pair : controls)
    {
        auto x = static_cast<void*>(&const_cast<string&>(pair.first));
        cvCreateTrackbar2(pair.first.c_str(), WINDOW_CONTROLS, &pair.second->value, pair.second->max,
                          &trackbarChanged, x);
    }
}


void colorThreshold(InputArray in, OutputArray out, cv::Scalar low, cv::Scalar high)
{
    Mat temp;
    cvtColor(in, temp, CV_BGR2HSV);
    
    // inRange converts to a 1 channel thing, so we use cvtColor
    // to get it to 3 channels again
    inRange(temp, low, high, temp);
    cvtColor(temp, temp,CV_GRAY2BGR);
    
    bitwise_and(temp, in, out);
}

void getContours(InputArray in, OutputArray out, OutputArrayOfArrays contours, int blockSize, double C)
{
    const double MAX_VALUE = 255.0f;
    Mat temp;
    Mat hierarchy; // used but ignored
    // Check http://docs.opencv.org/2.4/modules/imgproc/doc/structural_analysis_and_shape_descriptors.html?highlight=findcontours#findcontours
    // retrieves all of the contours and reconstructs a full hierarchy of nested contours.
    // This full hierarchy is built and shown in the OpenCV contours.c demo.
    int mode = CV_RETR_TREE;
    
    // compresses horizontal, vertical, and diagonal segments and leaves only their end points.
    // For example, an up-right rectangular contour is encoded with 4 points.
    int method = CV_CHAIN_APPROX_SIMPLE;
    
    // copy source to inout temp
    Mat drawtemp;
    in.copyTo(drawtemp);
    
    // gray scale: in --> temp
    cvtColor(in, temp, CV_BGR2GRAY);
    
    // threshold: temp --> temp
    adaptiveThreshold(temp, temp, MAX_VALUE, CV_ADAPTIVE_THRESH_GAUSSIAN_C,
                      CV_THRESH_BINARY, blockSize, C);
    
    findContours(temp, contours, hierarchy, mode, method);
    
    // draw contours
    Scalar color( rand()&255, rand()&255, rand()&255 );
    drawContours(drawtemp, contours, -1, color);
    
    // drawtemp --> out
    drawtemp.copyTo(out);
}

void getBoxesAndRects(vector<vector<Point>> contours, vector<RotatedRect> &rects, vector<vector<Point2f>> &boxes)
{
    const int MIN_AREA = 10;
    
    for (auto contour : contours)
    {
        if (contourArea(contour) > MIN_AREA)
        {
            cout << "contourArea: " << contourArea(contour) << endl;

            // add boxes
            RotatedRect minArea = minAreaRect(contour);
            vector<Point2f> pts(4);
            minArea.points(pts.data());
            boxes.push_back(pts);
            
            // add rects
            RotatedRect rect = minAreaRect(contour);
            rects.push_back(rect);
        }
    }
}
cv::Scalar getLowScalar()
{
    return cv::Scalar(controls[CTRL_HLOW]->value,
                      controls[CTRL_SLOW]->value,
                      controls[CTRL_VLOW]->value);
}
cv::Scalar getHighScalar()
{
    return cv::Scalar(controls[CTRL_HHIGH]->value,
                      controls[CTRL_SHIGH]->value,
                      controls[CTRL_VHIGH]->value);
}
RNG rng(12345);
int main()
{
    Scalar whiteColor(255,255,255);
    namedWindow("display");
    
    createTrackbars();
    
    VideoCapture vc(0);
    
    if (!vc.isOpened())
        throw "Problem opening camera!";
    
    Mat out, frame, temp;
    while(true)
    {
        vc >> frame;
        resize(frame, temp, Size_<int>(frame.cols/2, frame.rows/2));
        
        Mat drawing;
        temp.copyTo(drawing);
    
        int blockSize   = controls[CTRL_BLOCKSIZE]->value;
        int C           = controls[CTRL_C]->value;
        blockSize       = (blockSize%2)?blockSize: blockSize+1;
        C               = (C%2)?C: C+1;
        
        cv::Scalar low = getLowScalar();
        cv::Scalar high = getHighScalar();
        
        // color threshold
        // colorthreshold_output is BGR
        Mat colorthreshold_output;
        colorThreshold(temp, colorthreshold_output ,low, high);
        
        
        // from BGR to gray
        /// Convert image to gray and blur it
        Mat src_gray;
        cvtColor( colorthreshold_output, src_gray, CV_BGR2GRAY );
        blur( src_gray, src_gray, Size(3,3) );
        
        // threshold
        int thresh = 0;
        int max_thresh = 255;
        Mat threshold_output;
//        adaptiveThreshold(src_gray, threshold_output, max_thresh, CV_ADAPTIVE_THRESH_GAUSSIAN_C,
//                          CV_THRESH_BINARY, blockSize, C);
        threshold( src_gray, threshold_output, thresh, max_thresh, THRESH_BINARY );
        
        
        // Find contours
        vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;
        findContours( threshold_output, contours, hierarchy,
                     CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
        
        /// Approximate contours to polygons + get bounding rects and circles
        vector<vector<Point> > contours_poly( contours.size() );
        vector<Rect> boundRect( contours.size() );
        vector<Point2f>center( contours.size() );
        vector<float>radius( contours.size() );
        
        for( int i = 0; i < contours.size(); i++ )
        { approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
            boundRect[i] = boundingRect( Mat(contours_poly[i]) );
            minEnclosingCircle( (Mat)contours_poly[i], center[i], radius[i] );
        }
        
        
        /// Draw polygonal contour + bonding rects + circles
        Mat dr = Mat::zeros( threshold_output.size(), CV_8UC3 );
        for( int i = 0; i< contours.size(); i++ )
        {
            Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
            drawContours( dr, contours_poly, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
            
            //boundRect[i].
            rectangle( drawing, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
            circle( drawing, center[i], (int)radius[i], color, 2, 8, 0 );
        }
        
        imshow("display", drawing);
        
        waitKey(5);
        continue;
        
        
        // get contours
//        vector<vector<Point> > contours;
//        getContours(temp, out, contours, blockSize, C);
        
//        Mat src_gray1, threshold_output;
//        vector<Vec4i> hierarchy;
//        cvtColor(temp, src_gray, CV_HSV);
        
        
//        threshold( src_gray, threshold_output, thresh, max_thresh, THRESH_BINARY );
        
//        imshow("display", threshold_output);
        
        /// Find contours
        findContours( threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
//        
//        /// Approximate contours to polygons + get bounding rects and circles
//        vector<vector<Point> > contours_poly( contours.size() );
//        vector<Rect> boundRect( contours.size() );
//        vector<Point2f>center( contours.size() );
//        vector<float>radius( contours.size() );
//        
//        for( int i = 0; i < contours.size(); i++ )
//        { approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
//            boundRect[i] = boundingRect( Mat(contours_poly[i]) );
//            minEnclosingCircle( (Mat)contours_poly[i], center[i], radius[i] );
//        }
//
//    
//        /// Draw polygonal contour + bonding rects + circles
//        Mat dr = Mat::zeros( threshold_output.size(), CV_8UC3 );
//        for( int i = 0; i< contours.size(); i++ )
//        {
//            Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
//            drawContours( dr, contours_poly, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
//            rectangle( dr, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
//            circle( dr, center[i], (int)radius[i], color, 2, 8, 0 );
//        }
//        imshow("display", dr);
        continue;
          
        
        // filter the contours on area
        const int MIN_AREA = 10;
        vector<vector<Point>> filtered;
        vector<vector<Point2f>> boxes;
        
        for (auto it=contours.begin(); it != contours.end(); it++)
        {
            auto contour = *it;
            int n = (*it).size();
            int area = contourArea(*it);
            
            if (n>0 && area > MIN_AREA)
            {
                filtered.push_back(*it);
                RotatedRect rect = minAreaRect(contour);
                
                vector<Point2f> pts(4);
                rect.points(pts.data());
                boxes.push_back(pts);
            }
        }

        drawContours( drawing, filtered, -1, Scalar(12,120,180), 1, 8, vector<Vec4i>(), 0, Point() );

        // use rects (for SURF)
        // ...
        
        imshow("display", out);
        waitKey(10);
    }
    
    return 0;
}

//
//#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/imgproc/imgproc.hpp"
//#include <iostream>
//#include <stdio.h>
//#include <stdlib.h>
//
//using namespace cv;
//using namespace std;
//
//Mat src; Mat src_gray;
//int thresh = 100;
//int max_thresh = 255;
//RNG rng(12345);
//
///// Function header
//void thresh_callback(int, void* );
//
///** @function main */
//int main( int argc, char** argv )
//{
//    /// Load source image and convert it to gray
//    src = imread( argv[1], 1 );
//    
//    /// Convert image to gray and blur it
//    cvtColor( src, src_gray, CV_BGR2GRAY );
//    blur( src_gray, src_gray, Size(3,3) );
//    
//    /// Create Window
//    char* source_window = "Source";
//    namedWindow( source_window, CV_WINDOW_AUTOSIZE );
//    imshow( source_window, src );
//    
//    createTrackbar( " Threshold:", "Source", &thresh, max_thresh, thresh_callback );
//    thresh_callback( 0, 0 );
//    
//    waitKey(0);
//    return(0);
//}
//
///** @function thresh_callback */
//void thresh_callback(int, void* )
//{
//    Mat threshold_output;
//    vector<vector<Point> > contours;
//    vector<Vec4i> hierarchy;
//    
//    /// Detect edges using Threshold
//    threshold( src_gray, threshold_output, thresh, 255, THRESH_BINARY );
//    /// Find contours
//    findContours( threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
//    
//    /// Approximate contours to polygons + get bounding rects and circles
//    vector<vector<Point> > contours_poly( contours.size() );
//    vector<Rect> boundRect( contours.size() );
//    vector<Point2f>center( contours.size() );
//    vector<float>radius( contours.size() );
//    
//    for( int i = 0; i < contours.size(); i++ )
//    { approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
//        boundRect[i] = boundingRect( Mat(contours_poly[i]) );
//        minEnclosingCircle( (Mat)contours_poly[i], center[i], radius[i] );
//    }
//    
//    
//    /// Draw polygonal contour + bonding rects + circles
//    Mat drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );
//    for( int i = 0; i< contours.size(); i++ )
//    {
//        Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
//        drawContours( drawing, contours_poly, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
//        rectangle( drawing, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
//        circle( drawing, center[i], (int)radius[i], color, 2, 8, 0 );
//    }
//    
//    /// Show in a window
//    namedWindow( "Contours", CV_WINDOW_AUTOSIZE );
//    imshow( "Contours", drawing );
//}