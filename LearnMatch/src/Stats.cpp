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

#include "Stats.h"

using namespace std;

StatsTextLogger::StatsTextLogger(const std::string& path) :
	m_frameIdx(0),
	m_file(path.c_str(), fstream::out)
{
	m_file << "frame\tnum_keypoints\tnum_matches\thas_detection\tnum_inliers\tbest_index\ttime_detect\terror" << endl;
}

StatsTextLogger::~StatsTextLogger()
{
	m_file.close();
}

void StatsTextLogger::LogFrame(const Stats& stats)
{
	m_file << 
		m_frameIdx << "\t" << 
		stats.numKeypoints << "\t" << 
		stats.numMatches << "\t" << 
		stats.hasDetection << "\t" << 
		stats.numInliers << "\t" << 
		stats.bestSampleIndex << "\t" <<
		stats.timeDetect << "\t" <<
		stats.error << endl;
	++m_frameIdx;
}