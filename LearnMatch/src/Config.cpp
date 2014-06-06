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

#include "Config.h"

#include <fstream>
#include <iostream>
#include <cassert>
#include <stdlib.h>

using namespace std;

Config::Config(const std::string& path)
{
	SetDefaults();
	
	ifstream f(path.c_str());
	while (!(f.eof() || f.bad() || f.fail()))
	{
		char line[1000];
		f.getline(line, 1000);
		
		if (line[0] == '#') continue;
		
		char name[100];
		char value[900];
		int i = sscanf(line, "%s = %s", name, value);
		
		if (i == 2)
		{
			string s(name);
			
			if (s == "seed")
			{
				seed = atoi(value);
			}
			else if (s == "quietMode")
			{
				quietMode = (bool)atoi(value);
			}
			else if (s == "useVideo")
			{
				useVideo = (bool)atoi(value);
			}
			else if (s == "videoPath")
			{
				videoPath = string(value);
			}
			else if (s == "doLog")
			{
				doLog = (bool)atoi(value);
			}
			else if (s == "logPath")
			{
				logPath = string(value);
			}
			else if (s == "maxModelKeypoints")
			{
				maxModelKeypoints = atoi(value);
			}
			else if (s == "maxDetectKeypoints")
			{
				maxDetectKeypoints = atoi(value);
			}
			else if (s == "prosacIts")
			{
				prosacIts = atoi(value);
			}
			else if (s == "svmLambda")
			{
				svmLambda = atof(value);
			}
			else if (s == "svmNu")
			{
				svmNu = atof(value);
			}					
			else if (s == "svmBinaryComponents")
			{
				svmBinaryComponents = atoi(value);
			}
			else if (s == "enableLearning")
			{
				enableLearning = (bool)atoi(value);
			}
			else if (s == "enableBinaryWeightVector")
			{
				enableBinaryWeightVector = (bool)atoi(value);
			}
			else if (s == "lossFunction")
			{
				string lf(value);
				if (lf == Config::LossFunctionName(kLossFunctionNumInliers))
				{
					lossFunction = kLossFunctionNumInliers;
				}
				else if (lf == Config::LossFunctionName(kLossFunctionHomography))
				{
					lossFunction = kLossFunctionHomography;
				}
				else if (lf == Config::LossFunctionName(kLossFunctionHamming))
				{
					lossFunction = kLossFunctionHamming;
				}
				else if (lf == Config::LossFunctionName(kLossFunctionNone))
				{
					lossFunction = kLossFunctionNone;
				}
				else
				{
					cout << "unrecognised loss function: " << lf << endl;
					cout << "valid options are: (" 
						<< Config::LossFunctionName(kLossFunctionNumInliers) << ","
						<< Config::LossFunctionName(kLossFunctionHamming) << ","
						<< Config::LossFunctionName(kLossFunctionHomography) << ","
						<< Config::LossFunctionName(kLossFunctionNone) << ")" << endl;
					assert(false);
				}
			}
		}
	}
}

void Config::SetDefaults()
{
	seed = 0;
	quietMode = false;
	useVideo = false;
	videoPath = "";
	doLog = false;
	logPath = "log.txt";
	lossFunction = kLossFunctionNumInliers;
	maxModelKeypoints = 100;
	maxDetectKeypoints = 1000;
	maxMatchesPerModelKeypoint = 2; // hard-wired now
	prosacIts = 256;
	svmLambda = 0.1;
	svmNu = 1.0;
	svmBinaryComponents = 2;
	enableLearning = true;
	enableBinaryWeightVector = true;
}

string Config::LossFunctionName(LossFunction lf)
{
	string s;
	switch (lf)
	{
	case kLossFunctionNone:
		s = "none";
		break;	
	case kLossFunctionNumInliers:
		s = "numInliers";
		break;
	case kLossFunctionHomography:
		s = "homography";
		break;
	case kLossFunctionHamming:
		s = "hamming";
		break;
	default:
		assert(false);
	}
	return s;
}

ostream& operator<< (ostream& out, const Config& conf)
{
    out << "  seed                       = " << conf.seed << endl;
	out << "  quietMode                  = " << conf.quietMode << endl;
	out << "  useVideo                   = " << conf.useVideo << endl;
	out << "  videoPath                  = " << conf.videoPath << endl;
	out << "  doLog                      = " << conf.doLog << endl;
	out << "  logPath                    = " << conf.logPath << endl;
	out << "  lossFunction               = " << Config::LossFunctionName(conf.lossFunction) << endl;
	out << "  maxModelKeypoints          = " << conf.maxModelKeypoints << endl;
	out << "  maxDetectKeypoints         = " << conf.maxDetectKeypoints << endl;
	out << "  maxMatchesPerModelKeypoint = " << conf.maxMatchesPerModelKeypoint << endl;
	out << "  prosacIts                  = " << conf.prosacIts << endl;
	out << "  svmLambda                  = " << conf.svmLambda << endl;
	out << "  svmNu                      = " << conf.svmNu << endl;
	out << "  svmBinaryComponents        = " << conf.svmBinaryComponents << endl;
	out << "  enableLearning             = " << conf.enableLearning << endl;
	out << "  enableBinaryWeightVector   = " << conf.enableBinaryWeightVector << endl;
	return out;
}