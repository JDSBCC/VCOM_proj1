#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <stdio.h>

#include "Camera.h"
#include <iostream>
#include <string>


using namespace std;
using namespace cv;

void detectRoadInImage(string imageDir) {
	Mat image1 = imread(imageDir, 1);
	Mat image;
	resize(image1, image, Size(500, 500), 0, 0, INTER_LINEAR);
	Mat gray;
	cvtColor(image, gray, CV_BGR2GRAY);
	threshold(gray, gray, 100, 255, THRESH_BINARY);
	double t = 0;
	t = (double)cvGetTickCount();
	Mat Erode;
	erode(gray, Erode, Mat(), Point(2, 2), 7);
	Mat Dilate;
	dilate(gray, Dilate, Mat(), Point(2, 2), 7);
	threshold(Dilate, Dilate, 1, 50, THRESH_BINARY_INV);
	Mat path_trace(gray.size(), CV_8U, Scalar(0));
	path_trace = Erode + Dilate;

	Mat path;
	path_trace.convertTo(path, CV_32S);
	namedWindow("founded road");
	namedWindow("input image");

	watershed(image, path);
	path.convertTo(path, CV_8U);

	Mat road_found = path;
	road_found.convertTo(road_found, CV_8U);
	imshow("founded road", road_found);
	imshow("input image", image);
	t = (double)cvGetTickCount() - t;
	printf("road got detected in = %g ms\n", t / ((double)cvGetTickFrequency()*1000.));
	cout << endl << "cheers" << endl;
	imwrite("ROAD.jpg", image);
	imwrite("ROAD_DETECTED.jpg", road_found);
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