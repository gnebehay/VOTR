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

#ifndef IMAGE_REP_H
#define IMAGE_REP_H

#include "Rect.h"

#include <opencv/cv.h>
#include <vector>

#include <Eigen/Core>

class ImageRep
{
public:
	ImageRep(const cv::Mat& rImage, bool computeIntegral, bool computeIntegralHists, bool colour = false);
	
	int Sum(const IntRect& rRect, int channel = 0) const;
	void Hist(const IntRect& rRect, Eigen::VectorXd& h) const;
	
	inline const cv::Mat& GetImage(int channel = 0) const { return m_images[channel]; }
	inline const IntRect& GetRect() const { return m_rect; }

private:
	std::vector<cv::Mat> m_images;
	std::vector<cv::Mat> m_integralImages;
	std::vector<cv::Mat> m_integralHistImages;
	int m_channels;
	IntRect m_rect;
};

#endif
