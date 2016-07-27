//
//  test-color.hpp
//  FetchMyBeer
//
//  Created by Sami Farhat on 12/06/2016.
//
//

#ifndef test_color_hpp
#define test_color_hpp


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


// Color Stuff
// -----------


enum FMB_Colors {
  RED = 0,
  BLUE = 1,
  GREEN = 2
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

private:
  cv::Scalar lowColor;
  cv::Scalar highColor;

};

#endif /* test_color_hpp */
