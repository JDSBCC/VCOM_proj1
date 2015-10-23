#pragma once

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std;
using namespace cv;

class FindRoad
{
private:
	Mat src, src_gray, detected_edges, dst;
	int edgeThresh = 1;
	int lowThreshold;
	int const max_lowThreshold = 100;
	int ratio = 3;
	int kernel_size = 3;
	char* window_name = "Edge Map";
public:
	FindRoad(Mat src);
	~FindRoad();

	//algorithms
	void canny();
	void cannyThreshold();
};

