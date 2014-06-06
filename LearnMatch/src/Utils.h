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

#ifndef UTILS_H
#define UTILS_H

#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/imgproc/imgproc.hpp>

inline bool CompareKeypoints(const cv::KeyPoint& k1, const cv::KeyPoint& k2)
{
	return k1.response > k2.response;
}

std::vector<int> RandPerm(int n);

void DrawHomography(const cv::Mat& H, cv::Mat image, float w, float h, const CvScalar& colour = CV_RGB(0, 255, 0));

template <typename T>
unsigned int CountNonZero(const std::vector<T>& vec)
{
	unsigned int count = 0;
	for (unsigned int i = 0; i < vec.size(); ++i)
	{
		count += (unsigned int)(vec[i] != (T)0);
	}
	return count;
}

double HomographyPatchLoss(const cv::Mat& H1, const cv::Mat& H2, float w, float h);

double HomographyLoss(const cv::Mat& H1, const cv::Mat& H2);

double HomographyLoss(const cv::Mat& H);


#endif
