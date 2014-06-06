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

#ifndef BINARY_WEIGHT_VECTOR_H
#define BINARY_WEIGHT_VECTOR_H

#include "BinaryDescriptor.h"

#include <opencv2/core/core.hpp>
#include <Eigen/Core>
#include <vector>

template <int Length>
class BinaryWeightVector
{
public:
	BinaryWeightVector(int numComponents);

	void Update(const Eigen::VectorXd& w);
	double Dot(const BinaryDescriptor<Length>& descriptor);
	
	void Debug();

//private:
	int m_numComponents;
	double m_bias;
	double m_norm;
	std::vector< BinaryDescriptor<Length> > m_descriptors;
	std::vector<double> m_coeffs;	
};

#include "BinaryWeightVector.inl"

#endif
