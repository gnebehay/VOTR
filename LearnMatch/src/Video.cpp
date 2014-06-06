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

#include "Video.h"

#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

Video::Video(const std::string& path, bool write, const std::string& format) :
	m_frameIdx(0),
	m_write(write)
{
	m_formatString = path + "/" + format;
	if (write)
	{
		cout << "starting video recording to: " << path << endl;
	}
}


Video::~Video()
{
}


bool Video::WriteFrame(const Mat& frame)
{
	if (!m_write) return false;
	
	char buf[1024];
	sprintf(buf, m_formatString.c_str(), m_frameIdx);
	++m_frameIdx;
	
	return imwrite(buf, frame);
}

bool Video::ReadFrame(Mat& rFrame)
{
	if (m_write) return false;
	
	char buf[1024];
	sprintf(buf, m_formatString.c_str(), m_frameIdx);
	++m_frameIdx;
	
	rFrame = imread(buf, -1);
	
	return !rFrame.empty();
}
	
