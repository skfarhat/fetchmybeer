//
//  test-color.hpp
//  FetchMyBeer
//
//  Created by Sami Farhat on 12/06/2016.
//
//

#ifndef test_color_hpp
#define test_color_hpp

#include <opencv2/opencv.hpp>
#include <map>
#include <vector>
#include <string>
#include "Trackbar.hpp"

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

/** when set to non-zero this displays info on the console */
//#define CONSOLE_DEBUG 1

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

const char* DISPLAY         =  "display";
const char* COLOR_CONTROLS  =  "colorControls";
const char* WINDOW_CONTROLS =  "controls";

cv::RNG rng(12345);
Scalar whiteColor(255,255,255);

enum FMB_Colors {
  NONE = 0,
  BLUE = 1,
  GREEN = 2,
  RED = 3
} FMB_Colors;

/** 
 * @class HSV color range
 */
class FMB_ColorRange {
public:

  /** constructor */
  FMB_ColorRange(cv::Scalar _lowColor, cv::Scalar _highColor)
  {
    lowColor = _lowColor;
    highColor = _highColor;
  }

  Scalar getLowColor() { return lowColor; }
  Scalar getHighColor() { return highColor; }

private:
  Scalar lowColor;
  Scalar highColor;
  
};

/** 
 * @class Main 
 */
class Main {
 public:

  /** initialise */ 
  void init();

  /** thresholds the image for colors in the ranges given in controls dict */
  void colorThreshold(cv::InputArray in, cv::OutputArray out);

  /** start running (too vague I know) */
  void start();

  /** de-initialise */ 
  void deinit();

  int getColorRangesCount() { return colorRangesCount; }

  std::map<enum FMB_Colors, vector<FMB_ColorRange*>> getColorRanges()
  { return colorRanges; }

  map<string, Trackbar*> getControls() { return controls; }

  // TODO(sami): return const_iterator instead of the whole vector
  /** returns a vector of trackbars that control HSV ranges */
  vector<Trackbar*> getHSVTrackbars();

 private:

  struct TrackbarData {
    /** reference to main class, this is to allow access Main's functions inside 
     * trackbarChange */
    Main* main;

    /** name of the trackbar that incurred the change event */
    std::string name;
  };

  std::map<enum FMB_Colors, vector<FMB_ColorRange*>> colorRanges;

  map<string, Trackbar*> controls;

  /**
   * this variable facilitates accessing the right color-control keys from the
   * 'controls' map. It allows us to get the values for keys like HHIGH_1, HHIGH_2
   * the 1 and 2.. are known through this variable
   * The expected value fo 'colorRangesCount' is 2 because of the presence of RED
   * which needs two ranges in the HSV spectrum.
   */
  int colorRangesCount = -1;

  // -------
  // Methods
  // -------

  /** */
  void createTrackbars();

  /** */
  void initColorRanges();

};

#endif /* test_color_hpp */
