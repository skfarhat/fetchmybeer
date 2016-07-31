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
  /** @param userdata can be anything. Note that this is not data that is passed,
   * to trackbarChanged callback, just a pointer to any kind of data that the user
   * could want to store in the Trackbar */
  Trackbar(std::string _windowName, std::string _name,
           int _min, int _max, int _value,
           CvTrackbarCallback2 on_change = nullptr,
           void *userdata = nullptr);

  int getMin() { return min; }
  int getMax() { return max; }
  int getValue() { return value; }
  std::string getName() { return name; }
  void *getUserdata() { return userdata; }

  /** set the Trackbar value and adjust the QT slider */
  void setValue(int);
  /** */
  void setUserdata(void* _userdata) { userdata = _userdata; }
  /** set the trackbar value to the initial default value */
  void setToDefault();

 private:
  static std::map<std::string, Trackbar*> trackbars;

  std::string windowName;
  std::string name;
  int min;
  int max;
  int value;

  /** the trackbar's default value, set at creation */
  const int DEFAULT_VALUE;

  void *userdata;
};

#endif /* Trackbar_hpp */
