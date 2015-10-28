#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <stdio.h>

#include "Camera.h"
#include "Image.h"
#include "FindRoad.h"
#include <iostream>
#include <string>


using namespace std;
using namespace cv;

void detectRoadInImage(string imageDir) {
	Image image1 = Image(imageDir);

	FindRoad find = FindRoad(image1.getImage());
	find.algorithm();
	//find.houghTransformJoin();
	waitKey(0);
}

void detectRoadInCameraVideo() {
	Camera cam = Camera(0);
	cam.loadAndShow();
}

int main(int argc, const char** argv)
{
	int option = 0;
	string dir = "";

	do {
		cout << "###################################" << endl;
		cout << "          ROAD DETECTION           " << endl;
		cout << "###################################" << endl;

		cout << "What do yout want?" << endl;
		cout << "1. Open an image" << endl;
		cout << "2. Capture with camera" << endl;
		cout << "3. Exit" << endl;

		cin >> option;

		switch (option) {
		case 1:
			cout << "Dir: ";
			cin >> dir;
			detectRoadInImage(dir);
			break;
		case 2:
			detectRoadInCameraVideo();
			break;
		case 3:
			return 0;
		}

	} while (option != 3);

	return 0;
}