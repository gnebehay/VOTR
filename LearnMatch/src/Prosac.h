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

#ifndef PROSAC_H
#define PROSAC_H

#include <vector>

class PROSACParams
{
public:
	PROSACParams(int maxSamples = 1000, int seed = 123456) :
		m_maxSamples(maxSamples),
		m_seed(seed)
	{}
	
	int m_maxSamples;
	int m_seed;
};

template <class T>
class PROSACSample
{
public:
	std::vector<int> sample;
	std::vector<unsigned char> inliers;
	float score;
	int numInliers;
	T hypothesis;
	
	inline bool operator<(const PROSACSample<T>& other)
	{
		return score > other.score;
	}
};

template <class T>
class PROSAC
{
public:
	PROSAC(const PROSACParams& params);
	
	bool Compute();
	bool FastPolish();
	virtual bool Polish() { return FastPolish(); };
	
	
	inline int GetNumInliers() const { return m_bestNumInliers; }
	inline const std::vector<unsigned char>& GetInliers() const { return m_bestInliers; }
	inline const T& GetModel() const { return m_bestHypothesis; }
	inline const std::vector< PROSACSample<T> >& GetSamples() const { return m_samples; }
	inline int GetBestSampleIdx() const { return m_bestSampleIdx; }
	inline int GetNumSamples() const { return m_numSamples; }
	
protected:
	
	PROSACParams m_params;
	
	T m_bestHypothesis;
	float m_bestScore;
	std::vector<unsigned char> m_bestInliers;
	int m_bestNumInliers;
	int m_bestSampleIdx;
	
	int m_sampleSize;
	int m_dataSize;
	std::vector<int> m_sample;
	std::vector<unsigned char> m_inliers;
	std::vector<T> m_solutions;
	int m_numSamples;
	int m_setSize;
	
	std::vector< PROSACSample<T> > m_samples;

	// these should all be implemented by concrete classes
	virtual int GetSampleSize() = 0;
	virtual int GetDataSize() = 0;
	virtual bool ValidateSample(const std::vector<int>& sample) = 0;
	virtual bool SolveForHypothesis(const std::vector<int>& sample, std::vector<T>& rSolutions) = 0;
	virtual void ScoreHypothesis(const T& hypothesis, float& rScore, std::vector<unsigned char>& rInliers, int& rNumInliers) = 0;

private:
	
	void PreCompute();
	bool DrawValidSample(std::vector<int>& rSample);
	
};

#include "Prosac.inl"

#endif
