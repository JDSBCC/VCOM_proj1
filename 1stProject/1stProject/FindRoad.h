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
	double distance(double dX0, double dY0, double dX1, double dY1);
	void turnGray();
	void cut();

	//algorithms
	void houghTranform(Mat img);
	void probabilisticHoughTranform(Mat img);
	void houghTransformJoin();
	void algorithm();

	//draws
	void drawDetectedLines(Mat &image, Scalar color = Scalar(255));

	//line Simplification
	void lineSeparator();
	void lineStretch(vector<Vec4i> linesSeparated, Point & meanBegin, Point & meanEnd);

	//utilities
	double cross(Point v1, Point v2);
	bool getIntersectionPoint(Point a1, Point a2, Point b1, Point b2, Point & intPnt);
};

