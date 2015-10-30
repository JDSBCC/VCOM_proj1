#include "stdafx.h"
#include "FindRoad.h"
#include <math.h>


FindRoad::FindRoad(Mat src){
	this->src = src;
	resize(this->src, this->src, Size(500, 500), 0, 0, INTER_LINEAR); //resize to window size
}

FindRoad::FindRoad(String videoDir) {
	VideoCapture cap = VideoCapture(videoDir);

	namedWindow("video", 1);
	while (cap.isOpened()) {

		Mat frame;
		cap >> frame;

		src = frame;
		houghTransformJoinVideo();

		imshow("video", src);

		if (waitKey(30) >= 0) break;
	}
}


FindRoad::~FindRoad()
{
}

void FindRoad::cut() {
	Mat mask = Mat::zeros(src.size(), CV_8UC1);
	rectangle(mask, Point(0, src.size().height / 2), Point(src.size().width, src.size().height), Scalar(255, 255, 255), CV_FILLED);
	src.copyTo(cut_image, mask);
}

void FindRoad::houghTranform(Mat img) {

	int houghVote = 70;

	vector<Vec2f> lines;
	if (houghVote < 1 || lines.size() > 2) { // we lost all lines. reset
		houghVote = 70;
	}
	else { houghVote += 25; }
	while (lines.size() < 5 && houghVote > 0) {
		HoughLines(img, lines, 1, PI / 180, houghVote);
		houghVote -= 5;
	}
	Mat result(img.size(), CV_8U, Scalar(255));
	src.copyTo(result);

	//draw the lines
	vector<Vec2f>::const_iterator it = lines.begin();
	hough = Mat(src.size(), CV_8U, Scalar(0));
	while (it != lines.end()) {

		float rho = (*it)[0];   // first element is distance rho
		float theta = (*it)[1]; // second element is angle theta

		if (theta > 0.09 && theta < 1.35 || theta < 3.14 && theta > 1.80) { // filter to remove vertical and horizontal lines
			// point of intersection of the line with first row
			Point pt1(rho / cos(theta), 0);
			// point of intersection of the line with last row
			Point pt2((rho - result.rows*sin(theta)) / cos(theta), result.rows);
			// draw a line
			line(result, pt1, pt2, Scalar(255), 2);
			line(hough, pt1, pt2, Scalar(255), 2);
		}
		++it;
	}
}

void FindRoad::probabilisticHoughTranform(Mat img) {
	// Detect lines
	lines.clear();
	HoughLinesP(img, lines, 1, PI/180, 4, 60, 10);

	houghP = Mat(src.size(), CV_8U, Scalar(0));

	drawDetectedLines(houghP);
	lines.clear();
}

void FindRoad::houghTransformJoin() {

	//cut image
	cut();
	imshow("1. CutWindow", cut_image);
	
	//white and yellow segmentation
	cvtColor(cut_image, hsv_image, CV_BGR2HSV);
	inRange(hsv_image, Scalar(0, 0, 0), Scalar(180, 50, 255), white_lines);//white lines
	inRange(hsv_image, Scalar(18, 102, 204), Scalar(26, 255, 255), yellow_lines);//yellow lines
	bitwise_or(yellow_lines, white_lines, removeBack);
	cut_image.copyTo(removed, removeBack);
	imshow("2. RemoveBackgroundWindow", removed);

	//erode - dilate
	/*erode(removed, treshold_img, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	dilate(treshold_img, treshold_img, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	dilate(treshold_img, treshold_img, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	erode(treshold_img, treshold_img, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	imshow("ThresholdWindow", treshold_img);*/

	//turn image in a gray scale
	cvtColor(removed, src_gray, CV_BGR2GRAY);
	imshow("3. GrayWindow", src_gray);

	//canny algorithm
	//erode(src_gray, erode_img, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));
	dilate(src_gray, erode_img, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	medianBlur(erode_img, blur_img, 5);
	imshow("ErodeBlurWindow", blur_img);
	Canny(blur_img, detected_edges, 50, 400);
	imshow("4. CannyWindow", detected_edges);

	//Hough Transform algorithm
	houghTranform(detected_edges);
	imshow("5. HoughTransformWindow", hough);

	//Probabilistic Hough Transform algorithm
	probabilisticHoughTranform(detected_edges);
	imshow("6. ProbabilisticHoughTransformWindow", houghP);
	
	// bitwise AND of the two hough images
	bitwise_and(houghP, hough, houghP);
	Mat houghPinv=Mat(src.size(), CV_8U, Scalar(0));
	dst = Mat(src.size(), CV_8U, Scalar(0));
	threshold(houghP, houghPinv, 150, 255, THRESH_BINARY_INV); // threshold and invert to black lines
	imshow("7. HoughTransformJoinWindow", houghPinv);
	
	//canny
	Canny(houghPinv, detected_edges, 100, 350);
	lines.clear();
	HoughLinesP(detected_edges, lines, 1, PI / 180, 4, 60, 10);
	
	// Set probabilistic Hough parameters
	lineSeparator();
	drawDetectedLines(src);

	imshow("8. Final", src);
	lines.clear();
}

Mat FindRoad::houghTransformJoinVideo() {

	//cut image
	cut();

	//white and yellow segmentation
	cvtColor(cut_image, hsv_image, CV_BGR2HSV);
	inRange(hsv_image, Scalar(0, 0, 0), Scalar(180, 50, 255), white_lines);//white lines
	inRange(hsv_image, Scalar(18, 102, 204), Scalar(26, 255, 255), yellow_lines);//yellow lines
	bitwise_or(yellow_lines, white_lines, removeBack);
	cut_image.copyTo(removed, removeBack);

	//turn image in a gray scale
	cvtColor(removed, src_gray, CV_BGR2GRAY);

	//canny algorithm
	//erode(src_gray, erode_img, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));
	dilate(src_gray, erode_img, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	medianBlur(erode_img, blur_img, 5);
	Canny(blur_img, detected_edges, 50, 400);

	//Hough Transform algorithm
	houghTranform(detected_edges);

	//Probabilistic Hough Transform algorithm
	probabilisticHoughTranform(detected_edges);

	// bitwise AND of the two hough images
	bitwise_and(houghP, hough, houghP);
	Mat houghPinv = Mat(src.size(), CV_8U, Scalar(0));
	dst = Mat(src.size(), CV_8U, Scalar(0));
	threshold(houghP, houghPinv, 150, 255, THRESH_BINARY_INV); // threshold and invert to black lines

	//canny
	Canny(houghPinv, detected_edges, 100, 350);
	lines.clear();
	HoughLinesP(detected_edges, lines, 1, PI / 180, 4, 60, 10);

	// Set probabilistic Hough parameters
	lineSeparator();
	drawDetectedLines(src);
	lines.clear();

	return src;
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
	double temp_m = 0;
	double m = 0;

	vector<Vec4i> leftLines;
	vector<Vec4i> rightLines;

	vector<Vec4i>::const_iterator it2 = lines.begin();

	while (it2 != lines.end()) {

		Point pt1((*it2)[0], (*it2)[1]);
		Point pt2((*it2)[2], (*it2)[3]);

		Point vec(pt1.x-pt2.x, pt1.y-pt2.y);

		temp_m = m;
		m = (double)vec.y / (double)vec.x;

		count++;

		if (m > 0.0) {
			if(it2==lines.begin()){
				leftLines.push_back((*it2));
			} else if (abs(temp_m-m)<=1) {
				leftLines.push_back((*it2));
			}
		}else {
			if (it2 == lines.begin()) {
				rightLines.push_back((*it2));
			}
			else if (abs(temp_m - m) <= 1) {
				rightLines.push_back((*it2));
			}
		}
		++it2;
	}

	Point meanBeginLeft, meanBeginRight;
	Point meanEndLeft, meanEndRight;
	Point intPoint, downPointLeft, downPointRight;

	if (!leftLines.empty() && !rightLines.empty()) {
		lineStretchUp(leftLines, meanBeginLeft, meanEndLeft);
		lineStretchUp(rightLines, meanBeginRight, meanEndRight);

		getIntersectionPoint(meanBeginLeft, meanEndLeft, meanBeginRight, meanEndRight, intPoint);

		//extend lines
		lineStretchDown(meanBeginLeft, intPoint, meanBeginRight, intPoint, downPointLeft, downPointRight);

		vector<Vec4i> extendedLines;
		extendedLines.push_back(Vec4i(downPointLeft.x, downPointLeft.y, intPoint.x, intPoint.y));
		extendedLines.push_back(Vec4i(downPointRight.x, downPointRight.y, intPoint.x, intPoint.y));

		if (!extendedLines.empty()) {
			oldLines = extendedLines;
			lines.clear();
			lines = extendedLines;
		}

		//show intersection point
		circle(src, intPoint, 8, Scalar(0, 255, 255), 3);
	}else {
		if (!oldLines.empty()) {
			lines.clear();
			lines = oldLines;
		}

		//show intersection point
		circle(src, intPoint, 8, Scalar(0, 255, 255), 3);
	}
}

void FindRoad::lineStretchUp(vector<Vec4i> linesSeparated, Point & meanBegin, Point & meanEnd) {
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
}

void FindRoad::lineStretchDown(Point lup, Point ldown, Point rup, Point rdown, Point & dpl, Point & dpr) {
	double ml = (double)(lup.y - ldown.y) / (double)(lup.x - ldown.x);
	double bl = lup.y - ml*lup.x;
	double xl = (src.size().height - bl) / ml;
	double yl = bl;

	double mr = (double)(rup.y - rdown.y) / (double)(rup.x - rdown.x);
	double br = rup.y - mr*rup.x;
	double xr = (src.size().height - br) / mr;
	double yr = mr*src.size().width + br;

	if (xl>=0) {
		dpl = Point(xl, src.size().height);
	}else {
		dpl = Point(0, yl);
	}

	if (xr <= src.size().width ) {
		dpr = Point(xr, src.size().height);
	}
	else {
		dpr = Point(src.size().width, yr);
	}
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
