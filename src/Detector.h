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

#ifndef DETECTOR_H
#define DETECTOR_H

#include "Config.h"
#include "Model.h"
#include "Rect.h"
#include "LinearSVM.h"
#include "RobustHomography.h"
#include "BinaryWeightVector.h"
#include "Stats.h"

#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>

class Detector
{
public:
	Detector(const Config& config);
	~Detector();
	
	void Reset();
	bool SetModel(const cv::Mat& image, const IntRect& rect);
	void Detect(const cv::Mat& image, bool doUpdate = false);
	
	void Debug();
	
	inline bool HasModel() const { return m_pModel != 0; }
	inline bool HasDetection() const { return m_hasDetection; }
	inline const cv::Mat& GetH() const { return m_H; }
	inline cv::Mat GetDebugImage() const { return m_pModel ? m_pModel->GetDebugImage() : cv::Mat(); }
	inline void LogStats() { if (m_pLogger) m_pLogger->LogFrame(m_stats); }
	
	bool m_useClassifiers;
	bool m_useBinaryWeightVector;

	// temp
	Stats m_stats;
//private:
	const Config& m_config;
	Model* m_pModel;
	
	StatsLogger* m_pLogger;
	
	cv::Mat m_debugImage;
	
	bool m_hasDetection;
	cv::Mat m_H;
};

#endif
