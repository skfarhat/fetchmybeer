//
//  Matcher.hpp
//  FetchMyBeer
//
//  Created by Sami Farhat on 04/08/2016.
//
//

#ifndef matcher_hpp
#define matcher_hpp

//#include <opencv2/opencv.hpp>
#include <opencv2/opencv.hpp> 


class FMB_Matcher {
public:

  void match(cv::Mat img1, cv::Mat img2,
             cv::Mat out1, cv::Mat out2,
             int threshold, int minHessian);
private:

};

#endif /* matcher_hpp */
