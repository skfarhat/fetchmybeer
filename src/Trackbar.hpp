//  Copyright 2016 <Sami Farhat>
//  Trackbar.hpp
//  FetchMyBeer
//
//  Created by Sami Farhat on 30/07/2016.
//
//

#ifndef Trackbar_hpp
#define Trackbar_hpp
#include <opencv2/opencv.hpp>
#include <string>
#include <map>


class Trackbar {
 public:
  // Constructors
  Trackbar(std::string, std::string, int, int, int);

  Trackbar(std::string _windowName, std::string _name,
                     int _min, int _max, int _value,
                     CvTrackbarCallback2 on_change, void* userdata);

  int getMin() { return min; }
  int getMax() { return max; }
  int getValue() { return value; }

  void setValue(int);

  /** set the trackbar value to the initial default value */
  void setToDefault();

 private:
  std::string windowName;
  std::string name;
  int min;
  int max;
  int value;

  /** the trackbar's default value, set at creation */
  const int DEFAULT_VALUE;

  static std::map<std::string, Trackbar*> trackbars;
};

#endif /* Trackbar_hpp */
