//
//  main.cpp
//  FetchMyBeer
//
//  Created by Sami Farhat on 12/06/2016.
//
//

#include "main.hpp"
#include <iostream>


using namespace cv;
using std::cout;
using std::cerr;
using std::cin;
using std::endl;
using std::multimap;

// =============================================================================
// TODO:

// * show color thresholding in sepearate frame
// * threshold image based on some preset colors: red, blue, green
// BUG:

// * when we switch to blue, green, we need to default the (ranges 2 onwards..)

// =============================================================================

void* stringToVoidP(string str) {
  void *data = static_cast<void*>(const_cast<char*>(str.c_str()));
  return data;
}

/** expects data to be an integer referring to an index in the array ctrlKeys*/
void trackbarChanged(int value, void * data) {

  long index = (long) data;
  string key = ctrlKeys[index];
//  const char *cData = const_cast<const char*>(static_cast<char*>(data));
//  string sData = string(cData);
//  cout << "sData: " << sData << endl;
  // if colorThreshold is the control moved
  // adjust the other trackbars
  if (key.compare(CTRL_COLOR_THRESH)==0) {
    auto pair = colorRanges.find((enum FMB_Colors) value);
    vector<FMB_ColorRange*> ranges =  pair->second;
    cout << "ranges: " << ranges.size() << endl;
    for (int i = 0; i < ranges.size(); i++) {
      FMB_ColorRange *range = ranges[i];
      cv::Scalar lowColor = range->getLowColor();
      cv::Scalar highColor = range->getHighColor();

      controls["HLOW_" + std::to_string(i+1)]->setValue( lowColor.val[0]);
      controls["HHIGH_" + std::to_string(i+1)]->setValue(highColor.val[0]);
      controls["SLOW_" + std::to_string(i+1)]->setValue( lowColor.val[1]);
      controls["SHIGH_" + std::to_string(i+1)]->setValue(highColor.val[1]);
      controls["VLOW_" + std::to_string(i+1)]->setValue( lowColor.val[2]);
      controls["VHIGH_" + std::to_string(i+1)]->setValue(highColor.val[2]);

      // TODO: consider changing the window name to indicate which color is changed
    }
  }
}

void initColorRanges() {

  // Red
  std::vector<FMB_ColorRange*> redRanges;
  redRanges.push_back(new FMB_ColorRange(Scalar(0,100,100), Scalar(10,255,255)));
  redRanges.push_back(new FMB_ColorRange(Scalar(170,0,0), Scalar(180,255,255)));
  colorRanges.insert(std::make_pair(RED, redRanges));

  // Blue
  std::vector<FMB_ColorRange*> blueRanges;
  blueRanges.push_back(new FMB_ColorRange(Scalar(100,32,32), Scalar(127,255,255)));
  colorRanges.insert(std::make_pair(BLUE, blueRanges));

  // Green
  std::vector<FMB_ColorRange*> greenRanges;
  greenRanges.push_back(new FMB_ColorRange(Scalar(40,128, 64), Scalar(90,255,255)));
  colorRanges.insert(std::make_pair(GREEN, greenRanges));

}

void createTrackbars() {

  controls[CTRL_C] = new Trackbar(WINDOW_CONTROLS, CTRL_C,
                                  1, 255, 3,
                                  trackbarChanged,
                                  (void*)0);

  controls[CTRL_BLOCKSIZE]= new Trackbar(WINDOW_CONTROLS, CTRL_BLOCKSIZE,
                                         1, 255, 7,
                                         trackbarChanged,
                                         (void*)1);
  controls[CTRL_MIN_AREA]= new Trackbar(WINDOW_CONTROLS, CTRL_MIN_AREA,
                                        1, 10000, 1000,
                                        trackbarChanged,
                                        (void*)2);

  // 0 = None, 1 = Red, 2 = Blue, 3 = Green
  controls[CTRL_COLOR_THRESH] = new Trackbar(WINDOW_CONTROLS, CTRL_COLOR_THRESH,
                                             0, 3, 0,
                                             trackbarChanged,
                                             (void*)3);

  // set colorRangesCount to the number of ranges for colors
  // is in the colorRange (should be 2, but the design is flexible)
  for (auto pair : colorRanges) {
    int n = pair.second.size();
    if (n  > colorRangesCount) colorRangesCount = n;
  }

  for (int i = 1; i <= colorRangesCount; i++) {
    // H[i]_Low
    // H[i]_High
    string HSV[] = {"H", "S", "V"};
    for (string letter : HSV) {
      string iLow = letter + "LOW_" + std::to_string(i);
      string iHigh = letter + "HIGH_" + std::to_string(i);

      // acceptable vlues for HSV are
      // H(0-180), S(0-255), V(0-255)
      int max = (letter.compare("H")==0)? 180 : 255;
      // for first color range, set the default of low to 0
      // for the rest set default to 255, this is like saying they are disabled
      // unless the user explicitly changes them
      // (same analogy goes for high, but 0 <--> 255 )
      int defaultLow = (i==1)? 0 : 255;
      int defaultHigh = (i==1)? 255: 0;

      // insert trackbars
      controls[iLow] = new Trackbar(COLOR_CONTROLS, iLow, 0, max, defaultLow);
      controls[iHigh] = new Trackbar(COLOR_CONTROLS, iHigh, 0, max, defaultHigh);
    }
  }
}

void init() {
  // initialise map with RED, BLUE, GREEN HSV colors
  initColorRanges();

  // create windows to be used for display
  namedWindow(DISPLAY, WINDOW_NORMAL);

  namedWindow(WINDOW_CONTROLS, WINDOW_NORMAL);
  resizeWindow(WINDOW_CONTROLS, 640, 320);

  namedWindow(COLOR_CONTROLS, WINDOW_NORMAL);
  resizeWindow(COLOR_CONTROLS, 640, 320);

  createTrackbars();
}

/**
 * thresholds the image for color values between those specified
 * DEV: uses colorRanges and colorRangesCount
 */
void colorThreshold(InputArray in, OutputArray out) {
  Mat inHSV;
  cvtColor(in, inHSV, CV_BGR2HSV);
  for (int i = 1; i <= colorRangesCount; i++) {

    cv::Scalar lowScalar, highScalar;

    // get high & low HSV from controls
    int lowH = controls["HLOW_" + std::to_string(i)]->getValue();
    int highH = controls["HHIGH_" + std::to_string(i)]->getValue();
    int lowS = controls[ "SLOW_" + std::to_string(i)]->getValue();
    int highS = controls["SHIGH_" + std::to_string(i)]->getValue();
    int lowV = controls[ "VLOW_" + std::to_string(i)]->getValue();
    int highV = controls["VHIGH_" + std::to_string(i)]->getValue();

    cv::Scalar low(lowH, lowS, lowV);
    cv::Scalar high(highH, highS, highV);

    // inRange converts to a 1 channel thing, so we use cvtColor
    // to get it to 3 channels again
    Mat temp;
    inRange(inHSV, low, high, temp);
    cvtColor(temp, temp, CV_GRAY2BGR);
    bitwise_and(in, temp, temp);

    bitwise_or(temp, out, out);
  }
}

void getContours(InputArray in, OutputArray out,
                 OutputArrayOfArrays contours,
                 int blockSize, double C) {

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

void getBoxesAndRects(vector<vector<Point>> contours,
                      vector<RotatedRect> &rects,
                      vector<vector<Point2f>> &boxes) {
  const int MIN_AREA = 10;

  for (auto contour : contours) {
    if (contourArea(contour) > MIN_AREA) {

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

int main() {

  init();

  VideoCapture vc(0);

  if (!vc.isOpened())
    throw "Problem opening camera!";

  Mat out, frame, temp;
  while(true) {
    vc >> frame;
    resize(frame, temp, Size_<int>(frame.cols/2, frame.rows/2));

    Mat drawing;
    temp.copyTo(drawing);

    // get parameters from controls
    int blockSize   = controls[CTRL_BLOCKSIZE]->getValue();
    int C           = controls[CTRL_C]->getValue();
    blockSize       = (blockSize%2)?blockSize: blockSize+1;
    C               = (C%2)? C : C+1;
    int ctrlMinArea = controls[CTRL_MIN_AREA]->getValue();

    // color threshold
    // colorthreshold_output is BGRb
    Mat colorthreshold_output = Mat::ones(temp.size(), CV_8UC3);
    colorThreshold(temp, colorthreshold_output);
    imshow(DISPLAY, colorthreshold_output);
    waitKey(100);
    continue;
    // =========================================================================

    // from BGR to gray
    /// Convert image to gray and blur it
    Mat src_gray;
    cvtColor( colorthreshold_output, src_gray, CV_BGR2GRAY );
    blur( src_gray, src_gray, Size(3,3) );

    // threshold
    int max_thresh = 255;
    Mat threshold_output;
    adaptiveThreshold(src_gray, threshold_output, max_thresh, CV_ADAPTIVE_THRESH_GAUSSIAN_C,
                      CV_THRESH_BINARY, blockSize, C);
    //        threshold( src_gray, threshold_output, thresh, max_thresh, THRESH_BINARY );


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


    // DRAWING
    //
    // number of shapes (rectangles or circles) to display
    int nShape = 0;

    // shape to draw: true --> circles, false --> rectangles
    bool drawCircles = false;

    // foreach contour, get the bounding rectangle and the area
    // if the area is above some certain value
    for( int i = 0; i < contours.size(); i++ ) {
      approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
      Rect rect = boundingRect( Mat(contours_poly[i]) );
      float area  = rect.area();

      if (area > ctrlMinArea) {

        // 1) Circles
        minEnclosingCircle((Mat)contours_poly[i], center[nShape], radius[nShape]);

        // 2) Rectangles
        boundRect[nShape] = rect;

        nShape++;
      }
    }

    Mat dr = Mat::zeros(threshold_output.size(), CV_8UC3);
    if (drawCircles) {
      for( int i = 0; i< nShape; i++ ) {
        Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
        drawContours( dr, contours_poly, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
        circle( drawing, center[i], (int)radius[i], color, 2, 8, 0 );
      }
    }
    else {
      // DRAW RECTANGLES
      for( int i = 0; i< nShape; i++ ) {
        Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
        drawContours( dr, contours_poly, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
        rectangle( drawing, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
      }
    }
    
    imshow("display", drawing);
    //    imshow("color", colorthreshold_output);
    
    waitKey(100);
  }
  
  return 0;
}