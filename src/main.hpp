//
//  test-color.hpp
//  FetchMyBeer
//
//  Created by Sami Farhat on 12/06/2016.
//
//

#ifndef test_color_hpp
#define test_color_hpp

//
#define CTRL_HLOW         "hlow"
#define CTRL_HHIGH        "hhigh"
#define CTRL_VLOW         "vlow"
#define CTRL_VHIGH        "vhigh"
#define CTRL_SLOW         "slow"
#define CTRL_SHIGH        "shigh"
#define CTRL_C            "C"
#define CTRL_BLOCKSIZE    "BlockSize"
#define CTRL_MIN_AREA     "MinArea"   // minimum area that a contour must satisfy
#define CTRL_COLOR_THRESH "ColorThresh" // which color do we want to threshold against

#include <opencv2/opencv.hpp>
#include <map>
#include <vector>
#include <string>
#include "Trackbar.hpp"

using std::map;
using std::vector;
using std::string;
using cv::Scalar;

std::string ctrlKeys[] = {
  CTRL_C,
  CTRL_BLOCKSIZE,
  CTRL_MIN_AREA,
  CTRL_COLOR_THRESH
};

// Color Stuff
// -----------


const char* DISPLAY         =  "display";
const char* COLOR_CONTROLS  =  "colorControls";
const char* WINDOW_CONTROLS =  "controls";

enum FMB_Colors {
  NONE = 0, 
  BLUE = 1,
  GREEN = 2,
  RED = 3
} FMB_Colors;

/** HSV color range */
class FMB_ColorRange {
public:

  /** constructor */
  FMB_ColorRange(cv::Scalar _lowColor, cv::Scalar _highColor)
  {
    lowColor = _lowColor;
    highColor = _highColor;
  }

  cv::Scalar getLowColor() { return lowColor; }
  cv::Scalar getHighColor() { return highColor; }
  
private:
  cv::Scalar lowColor;
  cv::Scalar highColor;

};


cv::RNG rng(12345);
Scalar whiteColor(255,255,255);

map<string, Trackbar*> controls;
std::map<enum FMB_Colors, vector<FMB_ColorRange*>> colorRanges;
int colorRangesCount = -1;

#endif /* test_color_hpp */
