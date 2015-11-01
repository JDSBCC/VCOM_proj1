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

		cap >> src;
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

	//nr of votes
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

		if (theta > 0.09 && theta < 1.48 || theta < 3.14 && theta > 1.66) { // filter to remove vertical and horizontal lines
			// point of intersection of the line with first row
			Point pt1(rho / cos(theta), 0);
			// point of intersection of the line with last row
			Point pt2((rho - result.rows*sin(theta)) / cos(theta), result.rows);
			// draw a line
			line(result, pt1, pt2, Scalar(255),2);
			line(hough, pt1, pt2, Scalar(255),2);
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
}

void FindRoad::houghTransformJoin() {

	//cut image
	cut();
	imshow("1. CutWindow", cut_image);
	
	//white and yellow segmentation
	cvtColor(cut_image, hsv_image, CV_BGR2HSV);
	inRange(hsv_image, Scalar(0, 0, 0), Scalar(180, 50, 255), white_lines);//get white lines and the road area (gray colors)
	inRange(hsv_image, Scalar(18, 102, 204), Scalar(26, 255, 255), yellow_lines);//get yellow lines
	bitwise_or(yellow_lines, white_lines, removeBack);//join white and yellow lines and save the result on removeBack
	cut_image.copyTo(removed, removeBack);//intersect cut image with removeBack to get only the road image
	imshow("2. RemoveBackgroundWindow", removed);

	//turn image in a gray scale
	cvtColor(removed, src_gray, CV_BGR2GRAY);//convert image to a gray scaled image
	imshow("3. GrayWindow", src_gray);

	// dilate and blur image to remove noise and get more larger lines
	dilate(src_gray, erode_img, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	medianBlur(erode_img, blur_img, 5);
	imshow("4. DilateBlurWindow", blur_img);

	//detect lines with canny
	Canny(blur_img, detected_edges, 50, 400);
	imshow("5. CannyWindow", detected_edges);

	//Hough Transform algorithm
	houghTranform(detected_edges);
	imshow("6. HoughTransformWindow", hough);

	//Probabilistic Hough Transform algorithm
	probabilisticHoughTranform(detected_edges);
	imshow("7. ProbabilisticHoughTransformWindow", houghP);
	
	// bitwise AND of the two hough images
	bitwise_and(houghP, hough, houghP);
	Mat houghPinv=Mat(src.size(), CV_8U, Scalar(0));
	dst = Mat(src.size(), CV_8U, Scalar(0));
	threshold(houghP, houghPinv, 150, 255, THRESH_BINARY_INV); // threshold and invert to black lines
	imshow("8. HoughTransformJoinWindow", houghPinv);
	
	//get lines of the resulted image
	Canny(houghPinv, detected_edges, 100, 350);
	lines.clear();
	HoughLinesP(detected_edges, lines, 1, PI / 180, 4, 60, 10);
	
	//separate right and left lines
	lineSeparator();

	//draw the last image with lines
	drawDetectedLines(src);

	imshow("9. Final", src);
	lines.clear();
}

Mat FindRoad::houghTransformJoinVideo() {

	//cut image
	cut();

	//white and yellow segmentation
	cvtColor(cut_image, hsv_image, CV_BGR2HSV);
	inRange(hsv_image, Scalar(0, 0, 0), Scalar(180, 50, 255), white_lines);//get white lines and the road area (gray colors)
	inRange(hsv_image, Scalar(18, 102, 204), Scalar(26, 255, 255), yellow_lines);//get yellow lines
	bitwise_or(yellow_lines, white_lines, removeBack);//join white and yellow lines and save the result on removeBack
	cut_image.copyTo(removed, removeBack);//intersect cut image with removeBack to get only the road image

	//turn image in a gray scale
	cvtColor(removed, src_gray, CV_BGR2GRAY);//convert image to a gray scaled image

	// dilate and blur image to remove noise and get more larger lines
	dilate(src_gray, erode_img, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	medianBlur(erode_img, blur_img, 5);

	//detect lines with canny
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

	//get lines of the resulted image
	Canny(houghPinv, detected_edges, 100, 350);
	lines.clear();
	HoughLinesP(detected_edges, lines, 1, PI / 180, 4, 60, 10);

	//separate right and left lines
	lineSeparator();

	//draw the last image with lines
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
	double temp_m = 0;
	float m = 0;

	vector<Vec4i> leftLines;
	vector<Vec4i> rightLines;

	vector<Vec4i>::const_iterator it2 = lines.begin();

	//separate lines in left and right lines
	while (it2 != lines.end()) {

		Point pt1((*it2)[0], (*it2)[1]);
		Point pt2((*it2)[2], (*it2)[3]);

		Point vec(pt2.x - pt1.x, pt2.y - pt1.y);

		temp_m = m;
		m = (float)vec.y / (float)vec.x;

		if (m < 0) {//because y=0 is on the top of image
			if(it2==lines.begin() || abs(temp_m - m) <= 1){
				leftLines.push_back((*it2));
			}
		} else {
			if (it2 == lines.begin() || abs(temp_m - m) <= 1) {
				rightLines.push_back((*it2));
			}
		}
		++it2;
	}

	Point beginLeft, endLeft, beginRight, endRight;
	Point intPoint, downPointLeft, downPointRight;
	
	if (!leftLines.empty() && !rightLines.empty()) {//if one of them is empty we draw the last lines calculated
		vector<Vec4i> oldLeftResults = leftLines;
		vector<Vec4i> oldRightResults = rightLines;

		//select who have more white lines
		setLinesWithMoreWhitePixels(leftLines);
		setLinesWithMoreWhitePixels(rightLines);

		if (leftLines.empty() || rightLines.empty()) {//if there arent white lines on one of them we make an average of lines
			lineAverage(oldLeftResults, beginLeft, endLeft);
			lineAverage(oldRightResults, beginRight, endRight);
		}else {//else we get the lines who have more white
			beginLeft = Point(leftLines[0][0], leftLines[0][1]);
			endLeft = Point(leftLines[0][2], leftLines[0][3]);
			beginRight = Point(rightLines[0][0], rightLines[0][1]);
			endRight = Point(rightLines[0][2], rightLines[0][3]);
		}

		getIntersectionPoint(beginLeft, endLeft, beginRight, endRight, intPoint);//get the intersection (vanishing) point
		lineStretch(beginLeft, intPoint, beginRight, intPoint, downPointLeft, downPointRight);//stretch the line to the end of image

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

void FindRoad::lineAverage(vector<Vec4i> linesSeparated, Point & meanBegin, Point & meanEnd) {
	vector<Vec4i>::const_iterator it2 = linesSeparated.begin();

	Point sumBegin(0, 0), sumEnd(0, 0);
	meanBegin = Point(0, 0);
	meanEnd = Point(0, 0);
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

void FindRoad::lineStretch(Point lup, Point ldown, Point rup, Point rdown, Point & dpl, Point & dpr) {
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

void FindRoad::setLinesWithMoreWhitePixels(vector<Vec4i> & lines) {

	vector<Vec4i>::const_iterator it2 = lines.begin();

	int moreWhite = -1;
	Point pWhite1, pWhite2;


	while (it2 != lines.end()) {
		int countP1 = getColorCount(Point((*it2)[0], (*it2)[1]));
		int countP2 = getColorCount(Point((*it2)[2], (*it2)[3]));

		int countT = countP1 + countP2;

		if (moreWhite<countT) {
			moreWhite = countT;
			pWhite1 = Point((*it2)[0], (*it2)[1]);
			pWhite2 = Point((*it2)[2], (*it2)[3]);
		}
		++it2;
	}

	if (moreWhite != -1) {
		lines.clear();
		lines.push_back(Vec4i(pWhite1.x, pWhite1.y, pWhite2.x, pWhite2.y));
	}
}

int FindRoad::getColorCount(Point p) {
	int count = 0;
	for (int i = p.x - 10; i < p.x + 10; i++) {
		for (int j = p.y - 10; j < p.y + 10; j++) {
			if (hsv_image.at<Vec3b>(Point(i, j)).val[2]>204 && hsv_image.at<Vec3b>(Point(i, j)).val[2]<255 && 
				hsv_image.at<Vec3b>(Point(i, j)).val[1]>0 && hsv_image.at<Vec3b>(Point(i, j)).val[1]<51) {
				count++;
			}
		}
	}
	return count;
}
