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
                   int _min, int _max, int _value) {
  windowName = _windowName;
  name = _name;
  min = _min;
  max = _max;
  value = _value;

  cv::createTrackbar(name.c_str(), windowName, &value, max);
  Trackbar::trackbars[name] = this;
}

Trackbar::Trackbar(string _windowName, string _name,
                   int _min, int _max, int _value,
                   CvTrackbarCallback2 on_change, void* userdata) {
  windowName = _windowName;
  name = _name;
  min = _min;
  max = _max;
  value = _value;

  cvCreateTrackbar2(name.c_str(), windowName.c_str() , &value, max,
                    on_change, userdata);
}
void Trackbar::setValue(int _value) {
  value = _value;
  cv::setTrackbarPos(name, windowName,  value);
}
