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

#include "Sampler.h"
#include "Config.h"

#define _USE_MATH_DEFINES
#include <cmath>

using namespace std;

vector<FloatRect> Sampler::RadialSamples(FloatRect centre, int radius, int nr, int nt)
{
	vector<FloatRect> samples;
	
	FloatRect s(centre);
	float rstep = (float)radius/nr;
	float tstep = 2*(float)M_PI/nt;
	samples.push_back(centre);
	
	for (int ir = 1; ir <= nr; ++ir)
	{
		float phase = (ir % 2)*tstep/2;
		for (int it = 0; it < nt; ++it)
		{
			float dx = ir*rstep*cosf(it*tstep+phase);
			float dy = ir*rstep*sinf(it*tstep+phase);
			s.SetXMin(centre.XMin()+dx);
			s.SetYMin(centre.YMin()+dy);
			samples.push_back(s);
		}
	}
	
	return samples;
}

vector<FloatRect> Sampler::PixelSamples(FloatRect centre, int radius, bool halfSample)
{
	vector<FloatRect> samples;
	
	IntRect s(centre);
	samples.push_back(s);
	
	int r2 = radius*radius;
	for (int iy = -radius; iy <= radius; ++iy)
	{
		for (int ix = -radius; ix <= radius; ++ix)
		{
			if (ix*ix+iy*iy > r2) continue;
			if (iy == 0 && ix == 0) continue; // already put this one at the start
			
			int x = (int)centre.XMin() + ix;
			int y = (int)centre.YMin() + iy;
			if (halfSample && (ix % 2 != 0 || iy % 2 != 0)) continue;
			
			s.SetXMin(x);
			s.SetYMin(y);
			samples.push_back(s);
		}
	}
	
	return samples;
}