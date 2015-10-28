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

void detectRoadInVideo(string imageDir) {
	/*VideoCapture cap1 = VideoCapture(imageDir);
	Mat frame;

	while(&cap1.isOpened)

	cap1 >> frame;
	FindRoad find = FindRoad(frame);
	//find.algorithm();
	find.houghTransformJoin();
	waitKey(0);*/
}

void detectRoadInImage(string imageDir) {
	Image image1 = Image(imageDir);

	FindRoad find = FindRoad(image1.getImage());
	//find.algorithm();
	find.houghTransformJoin();
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
		cout << "2. Open a video" << endl;
		cout << "3. Capture with camera" << endl;
		cout << "4. Exit" << endl;

		cin >> option;

		switch (option) {
		case 1:
			cout << "Dir: ";
			cin >> dir;
			detectRoadInImage(dir);
			break;
		case 2:
			cout << "Dir: ";
			cin >> dir;
			detectRoadInVideo(dir);
			break;
		case 3:
			detectRoadInCameraVideo();
			break;
		case 4:
			return 0;
		}

	} while (option != 3);

	return 0;
}