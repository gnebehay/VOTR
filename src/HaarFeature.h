/* 
 * Struck: Structured Output Tracking with Kernels
 * 
 * Code to accompany the paper:
 *   Struck: Structured Output Tracking with Kernels
 *   Sam Hare, Amir Saffari, Philip H. S. Torr
 *   International Conference on Computer Vision (ICCV), 2011
 * 
 * Copyright (C) 2011 Sam Hare, Oxford Brookes University, Oxford, UK
 * 
 * This file is part of Struck.
 * 
 * Struck is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Struck is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Struck.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef HAAR_FEATURE_H
#define HAAR_FEATURE_H

#include "Rect.h"
#include "ImageRep.h"

#include <vector>

class Sample;

class HaarFeature
{
public:
	HaarFeature(const FloatRect& bb, int type);
	~HaarFeature();
	
	float Eval(const Sample& s) const;
	
private:
	FloatRect m_bb;
	std::vector<FloatRect> m_rects;
	std::vector<float> m_weights;
	float m_factor;
};

#endif
