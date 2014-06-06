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

#ifndef BINARY_DESCRIPTOR_H
#define BINARY_DESCRIPTOR_H

#include <Eigen/Core>

template <int Length>
class BinaryDescriptor
{
public:
	BinaryDescriptor();
	explicit BinaryDescriptor(const unsigned char* p);

	inline void SetData(const unsigned char* p);
	Eigen::VectorXd AsVector() const;

	inline const unsigned char* GetDataPtr() const { return (unsigned char*)m_data; }
	inline unsigned int GetBitCount() const { return m_bitCount; }

private:
	unsigned int m_data[Length/32];
	unsigned int m_bitCount;
};

#include "BinaryDescriptor.inl"

#endif
