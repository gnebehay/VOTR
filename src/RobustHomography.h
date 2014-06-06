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

#ifndef ROBUST_HOMOGRAPHY_H
#define ROBUST_HOMOGRAPHY_H

#include "Prosac.h"

#include <vector>
#include <opencv2/core/core.hpp>

class RobustHomography : public PROSAC<cv::Mat>
{
public:
	RobustHomography(float inlierThreshold, int maxMatches, int maxSamples = 1000);
	
	void SetDataPointers(const std::vector<cv::Point2f>* pPointsA, const std::vector<cv::Point2f>* pPointsB);
	void SetScores(const std::vector<double>* pScores);
	void SetModelInds(const std::vector<int>* pInds);
	
	virtual bool Polish();
	
	virtual int GetSampleSize() { return 4; }
	virtual int GetDataSize() { return m_dataSize; }
	virtual bool ValidateSample(const std::vector<int>& sample);
	virtual bool SolveForHypothesis(const std::vector<int>& sample, std::vector<cv::Mat>& rSolutions);
	virtual void ScoreHypothesis(const cv::Mat& hypothesis, float& rScore, std::vector<unsigned char>& rInliers, int& rNumInliers);
	
private:
	float m_inlierThreshold2;
	int m_dataSize;
	const std::vector<cv::Point2f>* m_pPointsA;
	const std::vector<cv::Point2f>* m_pPointsB;
	const std::vector<double>* m_pScores;
	const std::vector<int>* m_pModelInds;
	int m_maxMatches;
};


#endif