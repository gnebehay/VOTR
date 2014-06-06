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

#include <cstring>

typedef unsigned char uchar;

template <int Length>
BinaryDescriptor<Length>::BinaryDescriptor() :
	m_bitCount(0)
{
	memset(m_data, 0, Length/8);
}

template <int Length>
BinaryDescriptor<Length>::BinaryDescriptor(const unsigned char* p)
{
	SetData(p);
}

template <int Length>
inline void BinaryDescriptor<Length>::SetData(const unsigned char* p)
{
	memcpy(m_data, p, Length/8);
	// count set bits
    m_bitCount = 0;
	const int n = Length/32;
	if (n == 8)
	{
		// unroll 256-bit case
		m_bitCount += BitCount32(m_data[0]);
		m_bitCount += BitCount32(m_data[1]);
		m_bitCount += BitCount32(m_data[2]);
		m_bitCount += BitCount32(m_data[3]);
		m_bitCount += BitCount32(m_data[4]);
		m_bitCount += BitCount32(m_data[5]);
		m_bitCount += BitCount32(m_data[6]);
		m_bitCount += BitCount32(m_data[7]);
	}
	else
	{
		for (int i = 0; i < n; ++i)
		{
			m_bitCount += BitCount32(m_data[i]);
		}
	}
}

template <int Length>
Eigen::VectorXd BinaryDescriptor<Length>::AsVector() const
{
	Eigen::VectorXd x(Length);
	int ind = 0;
	const unsigned char* p = (const unsigned char*)m_data;
	for (int i = 0; i < Length/8; ++i, ind += 8)
	{
		unsigned char v = p[i];
		x(ind  ) = (double)((v   ) & (uchar)1) - 0.5;
		x(ind+1) = (double)((v>>1) & (uchar)1) - 0.5;
		x(ind+2) = (double)((v>>2) & (uchar)1) - 0.5;
		x(ind+3) = (double)((v>>3) & (uchar)1) - 0.5;
		x(ind+4) = (double)((v>>4) & (uchar)1) - 0.5;
		x(ind+5) = (double)((v>>5) & (uchar)1) - 0.5;
		x(ind+6) = (double)((v>>6) & (uchar)1) - 0.5;
		x(ind+7) = (double)((v>>7) & (uchar)1) - 0.5;
	}
	x.normalize();
	return x;
}
