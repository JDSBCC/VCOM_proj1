#pragma once

#include "stdafx.h"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>

using namespace cv;
using namespace std;


class Camera
{
private:
	VideoCapture cap;
	double width = 0;
	double height = 0;
public:
	Camera(int numCamera);
	~Camera();
	void loadAndShow();

	//get
	VideoCapture getVideoCapture();
	double getWidthOfFrames();
	double getHeightOfFrames();

	//set
	void setWidthOfFrames(double width);
	void setHeightOfFrames(double height);
};

