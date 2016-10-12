//
//  main-matcher.cpp
//  FetchMyBeer
//
//  Created by Sami Farhat on 04/08/2016.
//
//

#include <stdio.h>
#include "Matcher.hpp"
#include <opencv2/opencv.hpp>
#include "Trackbar.hpp"
#include <unistd.h>

using namespace cv;
using std::string;

static int _threshold = 50;
static int minHessian = 400;

using std::cerr;
using std::cout;
using std::endl;

void trackbarChanged(int value, void * data) {

  Trackbar *trackbar = (Trackbar*) data;
  if (trackbar->getName().compare("FAST-Thresh")==0)  {
    _threshold = value;
  }
  else if (trackbar->getName().compare("Min-Hessian")==0) {
    minHessian = value;
  }
}

int main(int argc, char *argv[]) {

  namedWindow("matcher1");
  namedWindow("matcher2");
  namedWindow("draw");
  namedWindow("controls", WINDOW_NORMAL);
  resizeWindow("controls", 500, 100);

  // create trackbar for threshold
  new Trackbar("controls", "FAST-Thresh", 0, 1000, _threshold, trackbarChanged, NULL);
  new Trackbar("controls", "Min-Hessian", 0, 1000, minHessian, trackbarChanged, NULL);



  string BUDWEISER = "res/budweiser.png";
  string KRON = "res/real-kron.jpg";

  // FIXME: this is hardcoded for now
  string basePath = "/Users/Sami/dev/projects/fetchmybeer/";
  string filename1 = BUDWEISER;
  //  string filename2 = "res/real-kron.jpg";
  Mat img1 = imread(basePath + filename1);
  //  Mat img2 = imread(basePath + filename2);
  //  resize(img2, img2, Size_<int>(img2.cols/2, img2.rows/2));

  // resize img1 by 50%
  resize(img1, img1, Size_<int>(img1.cols/2, img1.rows/2));

  FMB_Matcher *matcher = new FMB_Matcher();

  Mat img2;
  Mat out1, out2;
  img1.copyTo(out1);

  VideoCapture vc(0);
  if (!vc.isOpened())
    throw "Problem opening camera!";
  while(1) {
    vc >> img2;
    resize(img2, out2, Size_<int>(img2.cols/2, img2.rows/2));

    if (img2.empty()) continue;
    matcher->match(img1, img2, out1, out2, _threshold, minHessian);
    imshow("matcher1", out1);
    imshow("matcher2", out2);

    waitKey(30);

    // 0.5 sec
//    usleep(500000);
  }
  
  

  return 0;
}
