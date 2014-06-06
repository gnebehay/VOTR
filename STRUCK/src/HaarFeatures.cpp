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

#include "HaarFeatures.h"
#include "Config.h"

static const int kSystematicFeatureCount = 192;

HaarFeatures::HaarFeatures(const Config& conf)
{
	SetCount(kSystematicFeatureCount);
	GenerateSystematic();
}

void HaarFeatures::GenerateSystematic()
{
	float x[] = {0.2f, 0.4f, 0.6f, 0.8f};
	float y[] = {0.2f, 0.4f, 0.6f, 0.8f};
	float s[] = {0.2f, 0.4f};
	for (int iy = 0; iy < 4; ++iy)
	{
		for (int ix = 0; ix < 4; ++ix)
		{
			for (int is = 0; is < 2; ++is)
			{
				FloatRect r(x[ix]-s[is]/2, y[iy]-s[is]/2, s[is], s[is]);
				for (int it = 0; it < 6; ++it)
				{
					m_features.push_back(HaarFeature(r, it));
				}
			}
		}
	}
}

void HaarFeatures::UpdateFeatureVector(const Sample& s)
{
	for (int i = 0; i < m_featureCount; ++i)
	{
		m_featVec[i] = m_features[i].Eval(s);
	}
}
