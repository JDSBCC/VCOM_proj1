#include "stdafx.h"
#include "Camera.h"


Camera::Camera(int numCamera)
{
	cap = VideoCapture(numCamera);
}


Camera::~Camera()
{
}

void Camera::loadAndShow() {
	if (!getVideoCapture().isOpened()){
		cout << "Cannot open the video file" << endl;
		return;
	}

	setWidthOfFrames(cap.get(CV_CAP_PROP_FRAME_WIDTH));
	setHeightOfFrames(cap.get(CV_CAP_PROP_FRAME_HEIGHT));

	cout << "Frame size : " << width << " x " << height << endl;

	namedWindow("CameraCapture", CV_WINDOW_AUTOSIZE);

	while (1)
	{
		Mat frame;

		bool bSuccess = cap.read(frame);

		if (!bSuccess) //if not success, break loop
		{
			cout << "Cannot read a frame from video file" << endl;
			break;
		}

		imshow("CameraCapture", frame); //show the frame in "MyVideo" window

		if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
		{
			cout << "esc key is pressed by user" << endl;
			break;
		}
	}
}

VideoCapture Camera::getVideoCapture() {
	return cap;
}

double Camera::getWidthOfFrames() {
	return width;
}

double Camera::getHeightOfFrames() {
	return height;
}

void Camera::setWidthOfFrames(double width) {
	this->width = width;
}

void Camera::setHeightOfFrames(double height) {
	this->width = width;
}
