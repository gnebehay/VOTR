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

#ifndef RECT_H
#define RECT_H

#include <iostream>
#include <algorithm>

template <typename T>
class Rect
{
public:
	Rect() :
		m_xMin(0),
		m_yMin(0),
		m_width(0),
		m_height(0)
	{
	}
	
	Rect(T xMin, T yMin, T width, T height) :
		m_xMin(xMin),
		m_yMin(yMin),
		m_width(width),
		m_height(height)
	{
	}
	
	template <typename T2>
	Rect(const Rect<T2>& rOther) :
		m_xMin((T)rOther.XMin()),
		m_yMin((T)rOther.YMin()),
		m_width((T)rOther.Width()),
		m_height((T)rOther.Height())
	{
	}

	inline void Set(T xMin, T yMin, T width, T height)
	{
		m_xMin = xMin;
		m_yMin = yMin;
		m_width = width;
		m_height = height;
	}
	
	inline T XMin() const { return m_xMin; }
	inline void SetXMin(T val) { m_xMin = val; }
	inline T YMin() const { return m_yMin; }
	inline void SetYMin(T val) { m_yMin = val; }
	inline T Width() const { return m_width; }
	inline void SetWidth(T val) { m_width = val; }
	inline T Height() const { return m_height; }
	inline void SetHeight(T val) { m_height = val; }
	
	inline void Translate(T x, T y) { m_xMin += x; m_yMin += y; }

	inline T XMax() const { return m_xMin + m_width; }
	inline T YMax() const { return m_yMin + m_height; }
	inline float XCentre() const { return (float)m_xMin + (float)m_width / 2; }
	inline float YCentre() const { return (float)m_yMin + (float)m_height / 2; }
	inline T Area() const { return m_width * m_height; }
	
	template <typename T2>
	friend std::ostream& operator <<(std::ostream &rOS, const Rect<T2>& rRect);
	
	template <typename T2>
	float Overlap(const Rect<T2>& rOther) const;
	
	template <typename T2>
	bool IsInside(const Rect<T2>& rOther) const;

private:
	T m_xMin;
	T m_yMin;
	T m_width;
	T m_height;
};

typedef Rect<int> IntRect;
typedef Rect<float> FloatRect;

template <typename T>
std::ostream& operator <<(std::ostream &rOS, const Rect<T>& rRect)
{
	rOS << "[origin: (" << rRect.m_xMin << ", " << rRect.m_yMin << ") size: (" << rRect.m_width << ", " << rRect.m_height << ")]";
	return rOS;
}

template <typename T>
template <typename T2>
float Rect<T>::Overlap(const Rect<T2>& rOther) const
{
	float x0 = std::max((float)XMin(), (float)rOther.XMin());
	float x1 = std::min((float)XMax(), (float)rOther.XMax());
	float y0 = std::max((float)YMin(), (float)rOther.YMin());
	float y1 = std::min((float)YMax(), (float)rOther.YMax());
	
	if (x0 >= x1 || y0 >= y1) return 0.f;
	
	float areaInt = (x1-x0)*(y1-y0);
	return areaInt/((float)Area()+(float)rOther.Area()-areaInt);
}

template <typename T>
template <typename T2>
bool Rect<T>::IsInside(const Rect<T2>& rOther) const
{
	return (XMin()>=rOther.XMin()) && (YMin()>=rOther.YMin()) && (XMax()<=rOther.XMax()) && (YMax()<=rOther.YMax());
}

#endif
