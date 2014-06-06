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

#ifndef TIMER_H
#define TIMER_H

#ifdef _WIN32

#include <windows.h>
// these get defined by windows.h, so undefine them again
#undef min
#undef max

class Timer
{
public:
	Timer();
	
	void Reset();
	double GetSeconds() const;
	
private:
	LARGE_INTEGER m_startTime;
	LARGE_INTEGER m_frequency;
};

Timer::Timer()
{
	QueryPerformanceFrequency(&m_frequency);
	Reset();
}

inline void Timer::Reset()
{
	QueryPerformanceCounter(&m_startTime);
}

inline double Timer::GetSeconds() const
{
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	return ((double)(time.QuadPart-m_startTime.QuadPart))/(double)m_frequency.QuadPart;
}

#else

// TODO...
class Timer
{
public:	
	void Reset() {};
	double GetSeconds() const { return 0.0; }
};

#endif

#endif
