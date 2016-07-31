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
using std::to_string;

/* =============================================================================

 TODO(sami):
 -----------
 * consider changing the window name to indicate which color is changed

 BUGS:
 ------

 ==============================================================================*/

void Main::init() {
  // initialise map with RED, BLUE, GREEN HSV colors
  initColorRanges();

  // create windows to be used for display
  namedWindow(DISPLAY, WINDOW_NORMAL);

  // debug window
  namedWindow("debug", WINDOW_NORMAL);
  resizeWindow("debug", 640, 320);

  namedWindow(WINDOW_CONTROLS, WINDOW_NORMAL);
  resizeWindow(WINDOW_CONTROLS, 640, 320);

  namedWindow(COLOR_CONTROLS, WINDOW_NORMAL);
  resizeWindow(COLOR_CONTROLS, 640, 320);

  createTrackbars();
}

/* DEV NOTE: uses controls and colorRangesCount */
void Main::colorThreshold(InputArray in, OutputArray out) {
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

void Main::start() {
  VideoCapture vc(0);

  if (!vc.isOpened())
    throw "Problem opening camera!";

  Mat out, frame, temp;
  // count the number times the loop has looped
  // used to display the occasional console debug info
  int count = 0;
  while(true) {
#ifdef CONSOLE_DEBUG
    if (count++ > 50) {
      count = 0;
      for (auto pair : controls) {
        cout << pair.first << " --> " << pair.second->getValue() << endl;
      }
    }
#endif

    // get frame from camera and resize it
    vc >> frame;
    resize(frame, temp, Size_<int>(frame.cols/2, frame.rows/2));

    Mat drawing;
    temp.copyTo(drawing);

    // get parameters from controls
    int blockSize   = controls[CTRL_BLOCKSIZE]->getValue();
    int C           = controls[CTRL_C]->getValue();

    // insure blocksize is odd and of min value of 3
    blockSize       = (blockSize%2)?blockSize: blockSize+1;
    blockSize       = (blockSize < 3)? 3 : blockSize;

    // insure C is odd
    C               = (C%2)? C : C+1;
    int ctrlMinArea = controls[CTRL_MIN_AREA]->getValue();

    // color threshold
    // colorthreshold_output is BGRb
    Mat colorthreshold_output = Mat::ones(temp.size(), CV_8UC3);
    colorThreshold(temp, colorthreshold_output);
    imshow(DISPLAY, colorthreshold_output);

    // from BGR to gray
    /// Convert image to gray and blur it
    Mat src_gray;
    cvtColor( colorthreshold_output, src_gray, CV_BGR2GRAY );
    blur( src_gray, src_gray, Size(3,3) );

    // threshold
    int max_thresh = 255;
    Mat threshold_output;
    adaptiveThreshold(src_gray, threshold_output, max_thresh,
                      CV_ADAPTIVE_THRESH_GAUSSIAN_C,
                      CV_THRESH_BINARY, blockSize, C);
    //    threshold( src_gray, threshold_output, thresh, max_thresh, THRESH_BINARY );

    imshow("debug", threshold_output);

    waitKey(100);
    continue;
    // =========================================================================

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
        minEnclosingCircle((Mat)contours_poly[i], center[nShape],
                           radius[nShape]);

        // 2) Rectangles
        boundRect[nShape] = rect;

        nShape++;
      }
    }

    Mat dr = Mat::zeros(threshold_output.size(), CV_8UC3);
    if (drawCircles) {
      for( int i = 0; i< nShape; i++ ) {
        Scalar color = Scalar(
                              rng.uniform(0, 255),
                              rng.uniform(0,255),
                              rng.uniform(0,255) );
        drawContours(dr, contours_poly, i, color, 1, 8,
                     vector<Vec4i>(), 0, Point() );
        circle(drawing, center[i], (int)radius[i], color, 2, 8, 0 );
      }
    } else {
      // DRAW RECTANGLES
      for( int i = 0; i< nShape; i++ ) {
        Scalar color = Scalar(rng.uniform(0, 255),
                              rng.uniform(0,255),
                              rng.uniform(0,255));
        drawContours(dr, contours_poly, i, color, 1, 8,
                     vector<Vec4i>(), 0, Point());
        rectangle(drawing, boundRect[i].tl(), boundRect[i].br(),
                  color, 2, 8, 0);
      }
    }

    imshow(DISPLAY, drawing);

    waitKey(100);
  }
}

void Main::deinit() {

}

vector<Trackbar*> Main::getHSVTrackbars() {
  vector<Trackbar*> trackbars;
  for (int i = 1; i <= colorRangesCount; i++) {
    trackbars.push_back(controls["HLOW_" + std::to_string(i)]);
    trackbars.push_back(controls["HHIGH_" + std::to_string(i)]);
    trackbars.push_back(controls["SLOW_" + std::to_string(i)]);
    trackbars.push_back(controls["SHIGH_" + std::to_string(i)]);
    trackbars.push_back(controls["VLOW_" + std::to_string(i)]);
    trackbars.push_back(controls["VHIGH_" + std::to_string(i)]);
  }
  return trackbars;
}

/** expects data to be an integer referring to an index in the array ctrlKeys*/
void trackbarChanged(int value, void * data) {

  // TODO(sami): assert that data is of type Main*
  // ..

  // let's get references to some useful fields
  Trackbar *trackbar = (Trackbar*) data;
  Main* main = (Main*)trackbar->getUserdata();
  auto controls = main->getControls();
  auto colorRangesCount = main->getColorRangesCount();
  auto colorRanges = main->getColorRanges();

  // if colorThreshold is the control moved
  // adjust the other trackbars
  if (trackbar->getName().compare(CTRL_COLOR_THRESH)==0) {
    if (value == 0) {
      // TODO(sami): reset the color trackbars to their defaults

      auto colorTrackbars = main->getHSVTrackbars();
      for (Trackbar *t : colorTrackbars) {
        t->setToDefault();
      }
      return;
    }

    auto pair = colorRanges.find((enum FMB_Colors) value);
    if (pair == colorRanges.end()) {
      // we shouldn't be here, issues in initialising colorRanges ?
      throw "Could not find a colorRange for value " +
      std::to_string(value) + "\n";
    }

    for (int i = 0; i < colorRangesCount; i++) {
      vector<FMB_ColorRange*> ranges =  pair->second;
      if (i < ranges.size()) {
        FMB_ColorRange *range = ranges[i];
        cv::Scalar lowColor = range->getLowColor();
        cv::Scalar highColor = range->getHighColor();
        controls["HLOW_" + std::to_string(i+1)]->setValue( lowColor.val[0]);
        controls["HHIGH_" + std::to_string(i+1)]->setValue(highColor.val[0]);
        controls["SLOW_" + std::to_string(i+1)]->setValue( lowColor.val[1]);
        controls["SHIGH_" + std::to_string(i+1)]->setValue(highColor.val[1]);
        controls["VLOW_" + std::to_string(i+1)]->setValue( lowColor.val[2]);
        controls["VHIGH_" + std::to_string(i+1)]->setValue(highColor.val[2]);
      } else {
        // for the rest of the color-control trackbars, default them
        controls["HLOW_" + std::to_string(i+1)]->setToDefault();
        controls["HHIGH_" + std::to_string(i+1)]->setToDefault();
        controls["SLOW_" + std::to_string(i+1)]->setToDefault();
        controls["SHIGH_" + std::to_string(i+1)]->setToDefault();
        controls["VLOW_" + std::to_string(i+1)]->setToDefault();
        controls["VHIGH_" + std::to_string(i+1)]->setToDefault();
      }
    }
  }
}

void Main::initColorRanges() {
  // Red
  std::vector<FMB_ColorRange*> redRanges;
  redRanges.push_back(new FMB_ColorRange(Scalar(0,100,100),
                                         Scalar(10,255,255)));
  redRanges.push_back(new FMB_ColorRange(Scalar(170,0,0),
                                         Scalar(180,255,255)));
  colorRanges.insert(std::make_pair(RED, redRanges));

  // Blue
  std::vector<FMB_ColorRange*> blueRanges;
  blueRanges.push_back(new FMB_ColorRange(Scalar(100,102,32),
                                          Scalar(127,255,255)));
  colorRanges.insert(std::make_pair(BLUE, blueRanges));

  // Green
  std::vector<FMB_ColorRange*> greenRanges;
  greenRanges.push_back(new FMB_ColorRange(Scalar(14,36, 64),
                                           Scalar(95,171,255)));
  colorRanges.insert(std::make_pair(GREEN, greenRanges));
}

void Main::createTrackbars() {
  controls[CTRL_C] = new Trackbar(WINDOW_CONTROLS, CTRL_C,
                                  1, 255, 3,
                                  trackbarChanged, this);
  controls[CTRL_BLOCKSIZE]= new Trackbar(WINDOW_CONTROLS, CTRL_BLOCKSIZE,
                                         1, 255, 7,
                                         trackbarChanged, this);
  controls[CTRL_MIN_AREA]= new Trackbar(WINDOW_CONTROLS, CTRL_MIN_AREA,
                                        1, 10000, 1000,
                                        trackbarChanged, this);
  // 0 = None, 1 = Red, 2 = Blue, 3 = Green
  controls[CTRL_COLOR_THRESH] = new Trackbar(WINDOW_CONTROLS, CTRL_COLOR_THRESH,
                                             0, 3, 0,
                                             trackbarChanged, this);

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

      // acceptable values for HSV are
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

int main() {
  Main *main = new Main();
  
  main->init();
  
  main->start();
  
  main->deinit(); 
  
  return 0;
}