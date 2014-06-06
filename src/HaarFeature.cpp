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

#include "HaarFeature.h"
#include "Sample.h"

#include <cassert>
#include <iostream>

using namespace std;

HaarFeature::HaarFeature(const FloatRect& bb, int type) :
	m_bb(bb)
{
	assert(type < 6);
	
	switch (type)
	{
	case 0:
		{
			m_rects.push_back(FloatRect(bb.XMin(), bb.YMin(), bb.Width(), bb.Height()/2));
			m_rects.push_back(FloatRect(bb.XMin(), bb.YMin()+bb.Height()/2, bb.Width(), bb.Height()/2));
			m_weights.push_back(1.f);
			m_weights.push_back(-1.f);
			m_factor = 255*1.f/2;
			break;
		}
	case 1:
		{
			m_rects.push_back(FloatRect(bb.XMin(), bb.YMin(), bb.Width()/2, bb.Height()));
			m_rects.push_back(FloatRect(bb.XMin()+bb.Width()/2, bb.YMin(), bb.Width()/2, bb.Height()));
			m_weights.push_back(1.f);
			m_weights.push_back(-1.f);
			m_factor = 255*1.f/2;
			break;
		}
	case 2:
		{
			m_rects.push_back(FloatRect(bb.XMin(), bb.YMin(), bb.Width()/3, bb.Height()));
			m_rects.push_back(FloatRect(bb.XMin()+bb.Width()/3, bb.YMin(), bb.Width()/3, bb.Height()));
			m_rects.push_back(FloatRect(bb.XMin()+2*bb.Width()/3, bb.YMin(), bb.Width()/3, bb.Height()));
			m_weights.push_back(1.f);
			m_weights.push_back(-2.f);
			m_weights.push_back(1.f);
			m_factor = 255*2.f/3;
			break;
		}
	case 3:
		{
			m_rects.push_back(FloatRect(bb.XMin(), bb.YMin(), bb.Width(), bb.Height()/3));
			m_rects.push_back(FloatRect(bb.XMin(), bb.YMin()+bb.Height()/3, bb.Width(), bb.Height()/3));
			m_rects.push_back(FloatRect(bb.XMin(), bb.YMin()+2*bb.Height()/3, bb.Width(), bb.Height()/3));
			m_weights.push_back(1.f);
			m_weights.push_back(-2.f);
			m_weights.push_back(1.f);
			m_factor = 255*2.f/3;
			break;
		}
	case 4:
		{
			m_rects.push_back(FloatRect(bb.XMin(), bb.YMin(), bb.Width()/2, bb.Height()/2));
			m_rects.push_back(FloatRect(bb.XMin()+bb.Width()/2, bb.YMin()+bb.Height()/2, bb.Width()/2, bb.Height()/2));
			m_rects.push_back(FloatRect(bb.XMin(), bb.YMin()+bb.Height()/2, bb.Width()/2, bb.Height()/2));
			m_rects.push_back(FloatRect(bb.XMin()+bb.Width()/2, bb.YMin(), bb.Width()/2, bb.Height()/2));
			m_weights.push_back(1.f);
			m_weights.push_back(1.f);
			m_weights.push_back(-1.f);
			m_weights.push_back(-1.f);
			m_factor = 255*1.f/2;
			break;
		}
	case 5:
		{
			m_rects.push_back(FloatRect(bb.XMin(), bb.YMin(), bb.Width(), bb.Height()));
			m_rects.push_back(FloatRect(bb.XMin()+bb.Width()/4, bb.YMin()+bb.Height()/4, bb.Width()/2, bb.Height()/2));
			m_weights.push_back(1.f);
			m_weights.push_back(-4.f);
			m_factor = 255*3.f/4;
			break;
		}				
	}
}

HaarFeature::~HaarFeature()
{
}

float HaarFeature::Eval(const Sample& s) const
{
	const ImageRep& image = s.GetImage();
	const FloatRect& roi = s.GetROI();
	float value = 0.f;
	for (int i = 0; i < (int)m_rects.size(); ++i)
	{
		const FloatRect& r = m_rects[i];
		IntRect sampleRect((int)(roi.XMin()+r.XMin()*roi.Width()+0.5f), (int)(roi.YMin()+r.YMin()*roi.Height()+0.5f),
			(int)(r.Width()*roi.Width()), (int)(r.Height()*roi.Height()));
		value += m_weights[i]*image.Sum(sampleRect);
	}
	return value / (m_factor*roi.Area()*m_bb.Area());
}