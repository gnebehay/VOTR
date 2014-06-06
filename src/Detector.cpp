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

#include "Detector.h"
#include "Config.h"
#include "Model.h"

using namespace std;
using namespace cv;
using namespace Eigen;

Detector::Detector(const Config& config) :
	m_config(config),
	m_pModel(0),
	m_pLogger(0)
{
}


Detector::~Detector()
{
	Reset();
}


void Detector::Reset()
{
	if (m_pModel)
	{
		delete m_pModel;
		m_pModel = 0;
	}
	if (m_pLogger)
	{
		delete m_pLogger;
		m_pLogger = 0;
	}
	
	m_useClassifiers = m_config.enableLearning;
	m_useBinaryWeightVector = m_config.enableBinaryWeightVector;
}


bool Detector::SetModel(const Mat& image, const IntRect& rect)
{	
	Reset();
	
	if (m_config.doLog)
	{
		m_pLogger = new StatsTextLogger(m_config.logPath);
	}
	m_pModel = new Model(m_config, image, rect);
	
	cout << "initialised detector" << endl;
	
	return true;
}

void Detector::Detect(const cv::Mat& image, bool doUpdate)
{
	m_pModel->m_useClassifiers = m_useClassifiers;
	m_pModel->m_useBinaryWeightVector = m_useBinaryWeightVector;
	m_stats = Stats();
	m_hasDetection = m_pModel->Detect(image, m_H, doUpdate, &m_stats);
}

void Detector::Debug()
{
}
