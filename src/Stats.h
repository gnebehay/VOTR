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

#ifndef STATS_H
#define STATS_H

#include <string>
#include <fstream>

class Stats
{
public:
	Stats() :
		numKeypoints(0),
		numMatches(0),
		hasDetection(false),
		numInliers(0),
		timeDetect(0),
		bestSampleIndex(0),
		error(0.0)
	{}
	
	int numKeypoints;
	int numMatches;
	bool hasDetection;
	int numInliers;
	double timeDetect;
	int bestSampleIndex;
	float error;

};

class StatsLogger
{
public:
	
	virtual void LogFrame(const Stats& stats) = 0;
};

class StatsTextLogger : public StatsLogger
{
public:
	StatsTextLogger(const std::string& path);
	~StatsTextLogger();
	
	virtual void LogFrame(const Stats& stats);

private:
	int m_frameIdx;
	std::fstream m_file;
};

#endif