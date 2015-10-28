#pragma once

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#define PI 3.1415926

using namespace std;
using namespace cv;

class FindRoad
{
private:
	Mat src, src_gray, detected_edges, dst, hough, houghP, cuted_image, blur_img, erode_img, treshold_img;
	vector<Vec4i> lines;
public:
	FindRoad(Mat src);
	~FindRoad();
	void turnGray();
	void cut();

	//algorithms
	void houghTranform(Mat img);
	void probabilisticHoughTranform(Mat img);
	void houghTransformJoin();
	void algorithm();

	//draws
	void drawDetectedLines(Mat &image, Scalar color = Scalar(255));
};

