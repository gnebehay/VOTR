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

#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <ostream>

class Config
{
public:
	
	Config() { SetDefaults(); }
	Config(const std::string& path);
	
	
	enum LossFunction
	{
		kLossFunctionNone,
		kLossFunctionNumInliers,
		kLossFunctionHamming,
		kLossFunctionHomography
	};
	
	int seed;
	bool quietMode;
	bool useVideo;
	std::string videoPath;
	bool doLog;
	std::string logPath;
	LossFunction lossFunction;
	int maxModelKeypoints;
	int maxDetectKeypoints;
	int maxMatchesPerModelKeypoint;
	int prosacIts;
	double svmLambda;
	double svmNu;
	int svmBinaryComponents;
	bool enableLearning;
	bool enableBinaryWeightVector;
	
	static std::string LossFunctionName(LossFunction lf);

private:
	
	void SetDefaults();
	
};

std::ostream& operator<< (std::ostream& out, const Config& conf);

#endif