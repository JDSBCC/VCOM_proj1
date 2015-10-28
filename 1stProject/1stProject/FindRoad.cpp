#include "stdafx.h"
#include "FindRoad.h"


FindRoad::FindRoad(Mat src){
	this->src = src;
	resize(this->src, this->src, Size(500, 500), 0, 0, INTER_LINEAR); //resize to window size
}


FindRoad::~FindRoad()
{
}

void FindRoad::turnGray() {
	cvtColor(src, src_gray, CV_BGR2GRAY);
}

void FindRoad::cut() {
	Mat mask = Mat::zeros(src_gray.size(), CV_8UC1);
	rectangle(mask, Point(0, src_gray.size().height / 2), Point(src_gray.size().width, src_gray.size().height), Scalar(255, 255, 255), CV_FILLED);
	src_gray.copyTo(cuted_image, mask);
}

void FindRoad::algorithm() {

	//turn image in a gray scale
	cvtColor(src, src_gray, CV_BGR2GRAY);

	//Treshold to get the lines of street
	adaptiveThreshold(src_gray, treshold_img, 255, CV_THRESH_BINARY, CV_ADAPTIVE_THRESH_MEAN_C, 11, -1.5);
	imshow("1. TresholdWindow", treshold_img);

	//remove noise with blur
	//erode(treshold_img, erode_img, getStructuringElement(MORPH_RECT, Size(2, 2)));
	medianBlur(treshold_img, blur_img, 7);
	imshow("2. BlurWindow", blur_img);

	//canny algorithm
	Canny(blur_img, detected_edges, 50, 250);
	imshow("3. CannyWindow", detected_edges);

	//probabilistic hough transform algorithm
	probabilisticHoughTranform(detected_edges);
	imshow("4. HoughTransformWindow", houghP);

	//draw lines with original image
	Mat houghPinv = Mat(src.size(), CV_8U, Scalar(0));
	threshold(houghP, houghPinv, 150, 255, THRESH_BINARY_INV);

	Canny(houghPinv, detected_edges, 100, 350);
	lines.clear();
	HoughLinesP(detected_edges, lines, 1, PI / 180, 4, 60, 10);

	drawDetectedLines(src);
	imshow("5. Final", src);
	lines.clear();
}

void FindRoad::canny() {

	turnGray();

	Canny(src_gray, detected_edges, 50, 250);

	threshold(detected_edges, dst, 128, 255, THRESH_BINARY_INV);

	imshow("Canny Window", detected_edges);
}

void FindRoad::houghTranform(Mat img) {

	int houghVote = 200;

	vector<Vec2f> lines;
	if (houghVote < 1 || lines.size() > 2) { // we lost all lines. reset
		houghVote = 200;
	}
	else { houghVote += 25; }
	while (lines.size() < 5 && houghVote > 0) {
		HoughLines(img, lines, 1, PI / 180, houghVote);
		houghVote -= 5;
	}
	cout << houghVote << "\n";
	Mat result(src.size(), CV_8U, Scalar(255));
	src.copyTo(result);

	// Draw the lines
	vector<Vec2f>::const_iterator it = lines.begin();
	hough = Mat(src.size(), CV_8U, Scalar(0));
	while (it != lines.end()) {

		float rho = (*it)[0];   // first element is distance rho
		float theta = (*it)[1]; // second element is angle theta

		if (theta > 0.09 && theta < 1.48 || theta < 3.14 && theta > 1.66) { // filter to remove vertical and horizontal lines
			// point of intersection of the line with first row
			Point pt1(rho / cos(theta), 0);
			// point of intersection of the line with last row
			Point pt2((rho - result.rows*sin(theta)) / cos(theta), result.rows);
			// draw a white line
			line(result, pt1, pt2, Scalar(255), 2);
			line(hough, pt1, pt2, Scalar(255), 2);
		}

		//std::cout << "line: (" << rho << "," << theta << ")\n"; 
		++it;
	}

	// Display the detected line image
	imshow("Hough Transform", result);
}

void FindRoad::probabilisticHoughTranform(Mat img) {
	// Detect lines
	lines.clear();
	HoughLinesP(img, lines, 1, PI/180, 4, 60, 10);

	houghP = Mat(src.size(), CV_8U, Scalar(0));

	drawDetectedLines(houghP);

	imshow("Probabilistic Hough Transform", houghP);
}

void FindRoad::houghTransformJoin() {

	canny();
	houghTranform(detected_edges);
	probabilisticHoughTranform(detected_edges);
	
	// bitwise AND of the two hough images
	bitwise_and(houghP, hough, houghP);
	Mat houghPinv=Mat(src.size(), CV_8U, Scalar(0));
	dst = Mat(src.size(), CV_8U, Scalar(0));
	threshold(houghP, houghPinv, 150, 255, THRESH_BINARY_INV); // threshold and invert to black lines

	imshow("Hough Transform Join", houghPinv);
	
	//canny
	Canny(houghPinv, detected_edges, 100, 350);
	lines.clear();
	HoughLinesP(detected_edges, lines, 1, PI / 180, 4, 60, 10);
	
	// Set probabilistic Hough parameters
	drawDetectedLines(src);

	imshow("Final", src);
	lines.clear();
}

void FindRoad::drawDetectedLines(Mat &image, Scalar color) {

	// Draw the lines
	vector<Vec4i>::const_iterator it2 = lines.begin();

	while (it2 != lines.end()) {

		Point pt1((*it2)[0], (*it2)[1]);
		Point pt2((*it2)[2], (*it2)[3]);

		line(image, pt1, pt2, color, 2);
		++it2;
	}
}
