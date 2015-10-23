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
	Mat src, src_gray, detected_edges, dst, hough, houghP;
	vector<Vec4i> lines;
public:
	FindRoad(Mat src);
	~FindRoad();

	//algorithms
	void canny();
	void houghTranform();
	void probabilisticHoughTranform();
	void houghTransformJoin();

	//draws
	void drawDetectedLines(Mat &image, Scalar color = Scalar(255));
};

