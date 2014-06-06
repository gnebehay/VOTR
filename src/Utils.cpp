/* 
 * Code to accompany the paper:
 *   Efficient Online Structured Output Learning for Keypoint-Based Object Tracking
 *   Sam Hare, Amir Saffari, Philip H. S. Torr
 *   Computer Vision and Pattern Recognition (CVPR), 2012
 * 
 * Copyright (C) 2012 Sam Hare, Oxford Brookes University, Oxford, UK
 * 
 * This file is part of learnmatch.
 * 
 * learnmatch is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * learnmatch is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with learnmatch.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include "Utils.h"

#include <cstdlib>
#include <iostream>

#include <Eigen/Core>

using namespace std;
using namespace cv;

vector<int> RandPerm(int n)
{
	vector<int> p(n);
	for (int i = 0; i < n; ++i)
	{
		int j = rand() % (i+1);
		p[i] = p[j];
		p[j] = i;
	}
	return p;
}

void DrawHomography(const Mat& H, Mat image, float w, float h, const CvScalar& colour)
{
	vector<Point2f> corners;
	corners.push_back(Point2f(0.f, 0.f));
	corners.push_back(Point2f(w, 0.f));
	corners.push_back(Point2f(w, h));
	corners.push_back(Point2f(0.f, h));

	vector<Point2f> cornersProj(4);
	Mat C(corners);
	Mat Cproj(cornersProj);
	perspectiveTransform(C, Cproj, H);

	for (int i = 0; i < 4; ++i)
	{
		line(image, cornersProj[i], cornersProj[(i+1)%4], colour, 2);
	}
}

double HomographyPatchLoss(const Mat& H1, const Mat& H2, float w, float h)
{
	vector<Point2f> corners;
	corners.push_back(Point2f(0.f, 0.f));
	corners.push_back(Point2f(w, 0.f));
	corners.push_back(Point2f(w, h));
	corners.push_back(Point2f(0.f, h));
	Mat C(corners);
	
	vector<Point2f> cornersProj1(4);
	vector<Point2f> cornersProj2(4);
	Mat Cproj1(cornersProj1);
	Mat Cproj2(cornersProj2);
	perspectiveTransform(C, Cproj1, H1);
	perspectiveTransform(C, Cproj2, H2);
	
	double loss = 0.0;
	for (int i = 0; i < 4; ++i)
	{
		float dx = cornersProj1[i].x - cornersProj2[i].x;
		float dy = cornersProj1[i].y - cornersProj2[i].y;
		loss += sqrtf(dx*dx+dy*dy)/4;
	}
	return loss;
}

double HomographyLoss(const Mat& H1, const Mat& H2)
{
	try {
	Mat A = H1.inv()*H2;
	return HomographyLoss(A);
	}
	catch (const Exception& e)
	{
		return 10000.0;
	}
}

double HomographyLoss(const Mat& A)
{
	Matx31d corners[] = {
			Matx31d(-1.0, -1.0, 1.0),
			Matx31d(-1.0,  1.0, 1.0),
			Matx31d( 1.0,  1.0, 1.0),
			Matx31d( 1.0, -1.0, 1.0)
	};
	
	// convert from Mat to Matx -- is there an easier way?
	Matx33d H(
		A.at<double>(0,0), A.at<double>(0,1), A.at<double>(0,2),
		A.at<double>(1,0), A.at<double>(1,1), A.at<double>(1,2),
		A.at<double>(2,0), A.at<double>(2,1), A.at<double>(2,2));
	
	double distance = 0.0;
	for (int i = 0; i < 4; ++i)
	{
		const Matx31d& c = corners[i];
		Matx31d cp = H*c;
		double dx = c(0,0) - cp(0,0)/cp(2,0);
		double dy = c(1,0) - cp(1,0)/cp(2,0);
		distance += sqrt(dx*dx + dy*dy)/4;
	}
	return distance;
}