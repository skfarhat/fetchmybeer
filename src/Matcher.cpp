//
//  Matcher.cpp
//  FetchMyBeer
//
//  Created by Sami Farhat on 04/08/2016.
//
//

#include "Matcher.hpp"

#include "opencv2/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"
#include "opencv2/highgui.hpp"

using namespace cv;
using namespace cv::xfeatures2d;
using namespace std;

// ===========================================
// OVERVIEW
// --------
//
//
//
// Feature Detector + Extractor using SURF
// -->
// (desc1,desc2) FLAN-MATCHER
// -->
// matches
// -->
// filter-matches
// ===========================================

static void warpPerspectiveRand( const Mat& src, Mat& dst, Mat& H, RNG& rng )
{
  H.create(3, 3, CV_32FC1);
  H.at<float>(0,0) = rng.uniform( 0.8f, 1.2f);
  H.at<float>(0,1) = rng.uniform(-0.1f, 0.1f);
  H.at<float>(0,2) = rng.uniform(-0.1f, 0.1f)*src.cols;
  H.at<float>(1,0) = rng.uniform(-0.1f, 0.1f);
  H.at<float>(1,1) = rng.uniform( 0.8f, 1.2f);
  H.at<float>(1,2) = rng.uniform(-0.1f, 0.1f)*src.rows;
  H.at<float>(2,0) = rng.uniform( -1e-4f, 1e-4f);
  H.at<float>(2,1) = rng.uniform( -1e-4f, 1e-4f);
  H.at<float>(2,2) = rng.uniform( 0.8f, 1.2f);

  warpPerspective( src, dst, H, src.size() );
}

void FMB_Matcher::match(Mat img1, Mat img2, Mat out1, Mat out2,
                        int threshold = 10, int minHessian = 400) {

  bool isWarpPerspective = false;
  int ransacReprojThreshold = threshold;
  cout << "threshold = " << threshold << endl;

  Mat H12;
  RNG rng;
  if( isWarpPerspective )
    warpPerspectiveRand(img1, img2, H12, rng );
  else
    assert( !img2.empty()/* && img2.cols==img1.cols && img2.rows==img1.rows*/ );

  std::vector<KeyPoint> keypoints1, keypoints2;
  Mat desc1, desc2;

  //-- Step 1: Detect the keypoints using SURF Detector

  Ptr<SIFT> detector = SIFT::create(); // SURF::create( minHessian );
  std::vector<KeyPoint> keypoints_1, keypoints_2;

  detector->detect( img1, keypoints1 );
  detector->detect( img2, keypoints2 );

  detector->compute(img1, keypoints1, desc1);
  detector->compute(img2, keypoints2, desc2);

  // TODO: need to check for empty cases
  // ...

  //-- Step 3: Matching descriptor vectors using FLANN matcher
  FlannBasedMatcher matcher;
  std::vector< DMatch > matches;
  matcher.match( desc1, desc2, matches );


  //-- Quick calculation of max and min distances between keypoints
  double max_dist = 0; double min_dist = 100;
  for( int i = 0; i < desc1.rows; i++ )
  { double dist = matches[i].distance;
    if( dist < min_dist ) min_dist = dist;
    if( dist > max_dist ) max_dist = dist;
  }
  cout << "(min, max) : (" << min_dist << ", " << max_dist << ")" << endl;

  //-- Draw only "good" matches (i.e. whose distance is less than 2*min_dist,
  //-- or a small arbitary value ( 0.02 ) in the event that min_dist is very
  //-- small)
  //-- PS.- radiusMatch can also be used here.
  std::vector< DMatch > good_matches;
  for( int i = 0; i < desc1.rows; i++ ) {
    double max_val = max(2*min_dist, 0.02);
    //    cout << "max_val: " << max_val << endl;
    if( matches[i].distance <= max_val ) {
      good_matches.push_back( matches[i]);
    }
  }
  cout << "There are : " << good_matches.size() << endl;
  Mat img_matches;

  //-- Draw only "good" matches
  if (good_matches.size() < 13) {
    good_matches.clear();
  }

  drawMatches( img1, keypoints1, img2, keypoints2,
              good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
              vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
  imshow( "Good Matches", img_matches );
  return;

  // make an array of query indices and train indices from the matches found
  vector<int> queryIdxs( matches.size() ), trainIdxs( matches.size() );
  for( size_t i = 0; i < matches.size(); i++ ) {
    queryIdxs[i] = matches[i].queryIdx;
    trainIdxs[i] = matches[i].trainIdx;
  }

  // convert query and train indices and then find homography (in H12)
  if(!isWarpPerspective && ransacReprojThreshold >= 0 ) {
    cout << "< Computing homography (RANSAC)..." << endl;
    vector<Point2f> points1; KeyPoint::convert(keypoints1, points1, queryIdxs);
    vector<Point2f> points2; KeyPoint::convert(keypoints2, points2, trainIdxs);
    H12 = findHomography( Mat(points1), Mat(points2), CV_RANSAC, ransacReprojThreshold );
    cout << ">" << endl;
  }

  Mat drawImg;
  if( !H12.empty() ) // filter outliers
  {
    cout << "Filtering outliers" << endl;
    vector<char> matchesMask( matches.size(), 0 );
    vector<Point2f> points1; KeyPoint::convert(keypoints1, points1, queryIdxs);
    vector<Point2f> points2; KeyPoint::convert(keypoints2, points2, trainIdxs);
    Mat points1t; perspectiveTransform(Mat(points1), points1t, H12);

    double maxInlierDist = ransacReprojThreshold < 0 ? 3 : ransacReprojThreshold;
    for( size_t i1 = 0; i1 < points1.size(); i1++ ) {
      double normalised = norm(points2[i1] - points1t.at<Point2f>((int)i1,0));
      //      cout << "norm vs inlinerDist: " << normalised << " vs " << maxInlierDist << endl;
      if( normalised <= maxInlierDist ) // inlier
        matchesMask[i1] = 1;
    }
    // draw inliers
    drawMatches( img1, keypoints1, img2, keypoints2, matches, drawImg,
                Scalar(0, 255, 0), Scalar(255, 0, 0), matchesMask
#if DRAW_RICH_KEYPOINTS_MODE
                , DrawMatchesFlags::DRAW_RICH_KEYPOINTS
#endif
                );

#if DRAW_OUTLIERS_MODE
    cout << "Drawing outliers.." << endl;
    // draw outliers
    for( size_t i1 = 0; i1 < matchesMask.size(); i1++ )
      matchesMask[i1] = !matchesMask[i1];
    drawMatches( img1, keypoints1, img2, keypoints2, matches, drawImg,
                Scalar(255, 0, 0), Scalar(0, 0, 255), matchesMask,
                DrawMatchesFlags::DRAW_OVER_OUTIMG | DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
#endif

    cout << "Number of inliers: " << countNonZero(matchesMask) << endl;
  }
  imshow("draw" , drawImg);
  
  
}
