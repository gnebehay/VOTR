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

#include <iostream>
#include <cfloat>
#include <cmath>
#include <stdlib.h>

template <class T>
PROSAC<T>::PROSAC(const PROSACParams& params) :
	m_params(params)
{
}

template <class T>
void PROSAC<T>::PreCompute()
{
	m_sampleSize = GetSampleSize();
	m_dataSize = GetDataSize();
	m_sample.resize(m_sampleSize);
	m_bestHypothesis = T();
	m_bestScore = -FLT_MAX;
	m_bestSampleIdx = -1;
	m_bestInliers.resize(m_dataSize);
	m_inliers.resize(m_dataSize);
	m_bestNumInliers = 0;
}

template <class T>
bool PROSAC<T>::Compute()
{
	PreCompute();
	
	m_numSamples = 0;
	m_setSize = m_sampleSize;
	float Tn = 1.f;
	int Tndash = 1;
	
	int numFailed = 0;
	for (m_numSamples = 1; m_numSamples <= m_params.m_maxSamples; ++m_numSamples)
	{		
		if ((m_numSamples == Tndash || numFailed >= 5) && m_setSize < m_dataSize)
		{
			++m_setSize;
			numFailed = 0;
			float TnMinusOne = Tn;
			Tn *= ((float)m_setSize/(m_setSize-m_sampleSize));
			Tndash += (int)ceilf(Tn-TnMinusOne);
		}
		
		bool ok;
		ok = DrawValidSample(m_sample);
		if (!ok) 
		{
			//std::cout << "failed to draw a valid sample!" << std::endl;
			++numFailed;
			continue;
		}
		
		ok = SolveForHypothesis(m_sample, m_solutions);
		if (!ok)
		{
			++numFailed;
			std::cout << "SolveForHypothesis failed!" << std::endl;
			continue;
		}
		
		//++m_numSamples;
		numFailed = 0;
		
		PROSACSample<T> s;
		s.sample = m_sample;
		s.score = -FLT_MAX;
		//s.inliers.resize(m_dataSize, 0);
		//for (size_t i = 0; i < m_sampleSize; ++i)
		//	s.inliers[m_sample[i]] = 1;
		
		for (size_t i = 0; i < m_solutions.size(); ++i)
		{
			float score;
			int numInliers;
			ScoreHypothesis(m_solutions[i], score, m_inliers, numInliers);
			
			if (score > s.score)
			{
				s.score = score;
				s.numInliers = numInliers;
				s.inliers = m_inliers;
				s.hypothesis = m_solutions[i].clone(); // this is annoying
				//s.hypothesis = m_solutions[i];
			}
			
			if (score > m_bestScore)
			{
				m_bestScore = score;
				m_bestHypothesis = m_solutions[i].clone();
				m_bestNumInliers = numInliers;
				m_bestInliers = m_inliers;
				m_bestSampleIdx = (int)m_samples.size();
			}
		}
		
		m_samples.push_back(s);
	} 
	
	return true;
}

template <class T>
bool PROSAC<T>::FastPolish()
{
	std::vector<int> sample;
	sample.reserve(m_bestNumInliers);
	for (int i = 0; i < m_dataSize; ++i)
	{
		if (m_bestInliers[i]) sample.push_back(i);
	}
	
	bool ok = SolveForHypothesis(sample, m_solutions);
	if (!ok)
	{
		std::cout << "Polish SolveForHypothesis failed!" << std::endl;
		return false;
	}
	
	for (size_t i = 0; i < m_solutions.size(); ++i)
	{
		float score;
		int numInliers;
		ScoreHypothesis(m_solutions[i], score, m_inliers, numInliers);
		
		if (score > m_bestScore)
		{
			m_bestScore = score;
			m_bestHypothesis = m_solutions[i];
			m_bestNumInliers = numInliers;
			m_bestInliers = m_inliers;
		}
	}
	return true;
}

template <class T>
bool PROSAC<T>::DrawValidSample(std::vector<int>& rSample)
{
	if (m_setSize < m_sampleSize) return false;
	
	int tryCount = 0;
	const int kMaxTryCount = 20;
	bool valid = false;
	
	while(!valid && tryCount++ < kMaxTryCount); 
	{
		for (int i = 0; i < m_sampleSize-1; ++i)
		{
			rSample[i] = rand() % (m_setSize-1);

			for (int j = 0; j < i; ++j)
			{
				if (rSample[j] == rSample[i])
				{
					// All samples must be different - if we find a duplicate we decrement i to force another try
					i--; 
					break;
				}
			}
		}

		// final sample is always m_setSize-1
		rSample[m_sampleSize-1] = m_setSize-1;
		
		valid = ValidateSample(rSample);
	} 

	return valid;
}
