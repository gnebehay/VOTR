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

#include "BinaryUtils.h"

#include <cassert>
#include <iostream>

using namespace std;
using namespace cv;
using namespace Eigen;

template <int Length>
BinaryWeightVector<Length>::BinaryWeightVector(int numComponents) :
		m_numComponents(numComponents),
		m_norm(0.5*sqrt((double)Length)),
		m_descriptors(numComponents),
		m_coeffs(numComponents)
{
	// TODO: compile-time assert
	assert(Length % 8 == 0);
}

template <int Length>
void BinaryWeightVector<Length>::Update(const Eigen::VectorXd& w)
{
	m_bias = -0.5*w.sum();
	const int d = Length;
	const int db = Length/8;
	VectorXd residuals = w;
	VectorXd b(d);
	for (int i = 0; i < m_numComponents; ++i)
	{
		for (int j = 0; j < d; ++j)
		{
			b(j) = residuals(j) >= 0.0 ? 1.0 : -1.0;
		}

		double a = residuals.dot(b) / Length; // b.normSquared() = Length always
		m_coeffs[i] = a;
		
		residuals = residuals - a*b;
		
		uchar pp[Length/8];
		//uchar pn[Length/8];
		uchar* ppos = pp;
		//uchar* pneg = pn;
		for (int j = 0; j < db; ++j, ++ppos/*, ++pneg*/)
		{
    		uchar pos = 0;
    		//uchar neg = 0;
    		for (int k = 0; k < 8; ++k)
    		{
    			if (b(j*8+k) > 0.0)
    			{
    				pos |= 1 << k;
    			}
    			//else
    			//{
    			//	neg |= 1 << k;
    			//}
    		}
    		*ppos = pos;
    		//*pneg = neg;
		}

		m_descriptors[i].SetData(pp);
	}
}

template <int Length>
double BinaryWeightVector<Length>::Dot(const BinaryDescriptor<Length>& descriptor)
{
	double total = m_bias;
	for (int i = 0; i < m_numComponents; ++i)
	{
		total += m_coeffs[i]*(2*(int)DotProduct(m_descriptors[i], descriptor) - (int)descriptor.GetBitCount());
	}
	return total/m_norm;
}

template <int Length>
void BinaryWeightVector<Length>::Debug()
{
	for (int i = 0; i < m_numComponents; ++i)
	{
		cout << m_coeffs[i] << " ";
	}
	cout << endl;
}
