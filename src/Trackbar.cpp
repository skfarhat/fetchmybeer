//  Copyright 2016 <Sami Farhat>
//  Trackbar.cpp
//  FetchMyBeer
//
//  Created by Sami Farhat on 30/07/2016.
//
//

#include "Trackbar.hpp"

using std::string;

std::map<std::string, Trackbar*> Trackbar::trackbars;

Trackbar::Trackbar(string _windowName, string _name,
                   int _min, int _max, int _value,
                   CvTrackbarCallback2 on_change,
                   void *_userdata) :
DEFAULT_VALUE(_value) {
  windowName = _windowName;
  name = _name;
  min = _min;
  max = _max;
  value = _value;
  userdata = _userdata;

  if (on_change == nullptr) {
    cv::createTrackbar(name.c_str(), windowName, &value, max);
  } else {
    cvCreateTrackbar2(name.c_str(), windowName.c_str() , &value, max,
                      on_change, reinterpret_cast<void*>(this));
  }
}
void Trackbar::setValue(int _value) {
  value = _value;
  cv::setTrackbarPos(name, windowName,  value);
}

void Trackbar::setToDefault() {
  value = DEFAULT_VALUE;
}
