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

#include "HistogramFeatures.h"
#include "Config.h"
#include "Sample.h"
#include "Rect.h"

#include <iostream>

using namespace Eigen;
using namespace cv;
using namespace std;

static const int kNumBins = 16;
static const int kNumLevels = 4;
static const int kNumCellsX = 3;
static const int kNumCellsY = 3;

HistogramFeatures::HistogramFeatures(const Config& conf)
{
	int nc = 0;
	for (int i = 0; i < kNumLevels; ++i)
	{
		//nc += 1 << 2*i;
		nc += (i+1)*(i+1);
	}
	SetCount(kNumBins*nc);
	cout << "histogram bins: " << GetCount() << endl;
}

void HistogramFeatures::UpdateFeatureVector(const Sample& s)
{
	IntRect rect = s.GetROI(); // note this truncates to integers
	//cv::Rect roi(rect.XMin(), rect.YMin(), rect.Width(), rect.Height());
	//cv::resize(s.GetImage().GetImage(0)(roi), m_patchImage, m_patchImage.size());
	
	m_featVec.setZero();
	VectorXd hist(kNumBins);
	
	int histind = 0;
	for (int il = 0; il < kNumLevels; ++il)
	{
		int nc = il+1;
		float w = s.GetROI().Width()/nc;
		float h = s.GetROI().Height()/nc;
		FloatRect cell(0.f, 0.f, w, h);
		for (int iy = 0; iy < nc; ++iy)
		{
			cell.SetYMin(s.GetROI().YMin()+iy*h);
			for (int ix = 0; ix < nc; ++ix)
			{
				cell.SetXMin(s.GetROI().XMin()+ix*w);
				s.GetImage().Hist(cell, hist);
				m_featVec.segment(histind*kNumBins, kNumBins) = hist;
				++histind;
			}
		}
	}
	m_featVec /= histind;
}
