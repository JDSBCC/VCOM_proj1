#include "stdafx.h"
#include "FindRoad.h"
#include <math.h>


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
	//Mat dilate_img;
	//dilate(treshold_img, dilate_img, getStructuringElement(MORPH_RECT, Size(2, 2)));
	//erode(dilate_img, erode_img, getStructuringElement(MORPH_RECT, Size(2, 2)));
	//imshow("1.2. DilateWindow", treshold_img);
	//medianBlur(treshold_img, blur_img, 5);
	//imshow("2. BlurWindow", blur_img);

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

	//draw the lines
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
}

void FindRoad::probabilisticHoughTranform(Mat img) {
	// Detect lines
	lines.clear();
	HoughLinesP(img, lines, 1, PI/180, 4, 60, 10);

	houghP = Mat(src.size(), CV_8U, Scalar(0));

	drawDetectedLines(houghP);
}

void FindRoad::houghTransformJoin() {

	//turn image in a gray scale
	turnGray();

	//canny algorithm
	Canny(src_gray, detected_edges, 50, 250);
	imshow("1. CannyWindow", detected_edges);

	//Hough Transform algorithm
	houghTranform(detected_edges);
	imshow("2. HoughTransformWindow", hough);

	//Probabilistic Hough Transform algorithm
	probabilisticHoughTranform(detected_edges);
	imshow("3. ProbabilisticHoughTransformWindow", houghP);
	
	// bitwise AND of the two hough images
	bitwise_and(houghP, hough, houghP);
	Mat houghPinv=Mat(src.size(), CV_8U, Scalar(0));
	dst = Mat(src.size(), CV_8U, Scalar(0));
	threshold(houghP, houghPinv, 150, 255, THRESH_BINARY_INV); // threshold and invert to black lines
	imshow("4. HoughTransformJoinWindow", houghPinv);
	
	//canny
	Canny(houghPinv, detected_edges, 100, 350);
	lines.clear();
	HoughLinesP(detected_edges, lines, 1, PI / 180, 4, 60, 10);
	
	// Set probabilistic Hough parameters
	lineSeparator();
	drawDetectedLines(src);

	imshow("5. Final", src);
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

void FindRoad::lineSeparator() {
	int count = 0;

	vector<Vec4i> leftLines;
	vector<Vec4i> rightLines;

	vector<Vec4i>::const_iterator it2 = lines.begin();

	while (it2 != lines.end()) {

		Point pt1((*it2)[0], (*it2)[1]);
		Point pt2((*it2)[2], (*it2)[3]);

		Point vec(pt1.x-pt2.x, pt1.y-pt2.y);

		double m = (double)vec.y / (double)vec.x;

		count++;

		if (m > 0.0) {
			cout << "left = " << m << " count = " << count << endl;
			leftLines.push_back((*it2));
		}else {
			cout << "right = " << m << " count = " << count << endl;
			rightLines.push_back((*it2));
		}

		++it2;
	}

	Point meanBeginLeft, meanBeginRight;
	Point meanEndLeft, meanEndRight;
	Point intPoint;

	cout << "left" << endl;
	lineStretch(leftLines, meanBeginLeft, meanEndLeft);
	cout << "right" << endl;
	lineStretch(rightLines, meanBeginRight, meanEndRight);

	getIntersectionPoint(meanBeginLeft, meanEndLeft, meanBeginRight, meanEndRight, intPoint);
	cout << "intPoint => " << "x = " << intPoint.x << "; y = " << intPoint.y << endl;

	vector<Vec4i> extendedLines;
	extendedLines.push_back(Vec4i(meanBeginLeft.x, meanBeginLeft.y, intPoint.x, intPoint.y));
	extendedLines.push_back(Vec4i(meanBeginRight.x, meanBeginRight.y, intPoint.x, intPoint.y));

	if (!extendedLines.empty())
		lines = extendedLines;
}

void FindRoad::lineStretch(vector<Vec4i> linesSeparated, Point & meanBegin, Point & meanEnd) {
	vector<Vec4i>::const_iterator it2 = linesSeparated.begin();

	Point sumBegin(0,0), sumEnd(0,0);
	meanBegin=Point(0, 0);
	meanEnd=Point(0, 0);
	int count = 0;

	while (it2 != linesSeparated.end()) {

		Point pt1((*it2)[0], (*it2)[1]);
		Point pt2((*it2)[2], (*it2)[3]);

		sumBegin.x += pt1.x;
		sumBegin.y += pt1.y;
		sumEnd.x += pt2.x;
		sumEnd.y += pt2.y;

		count++;
		++it2;
	}

	meanBegin.x = sumBegin.x / count;
	meanBegin.y = sumBegin.y / count;
	meanEnd.x = sumEnd.x / count;
	meanEnd.y = sumEnd.y / count;

	cout <<"Begin => "<< "x = " << meanBegin.x << "; y = " << meanBegin.y << endl;
	cout <<"End => "<< "x = " << meanEnd.x << "; y = " << meanEnd.y << endl;
}

double FindRoad::cross(Point v1, Point v2) {
	return v1.x*v2.y - v1.y*v2.x;
}

bool FindRoad::getIntersectionPoint(Point a1, Point a2, Point b1, Point b2, Point & intPnt) {
	Point p = a1;
	Point q = b1;
	Point r(a2 - a1);
	Point s(b2 - b1);

	if (cross(r, s) == 0) { return false; }

	double t = cross(q - p, s) / cross(r, s);

	intPnt = p + t*r;

	return true;
}
