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

#ifndef LINEAR_SVM_H
#define LINEAR_SVM_H

#include <Eigen/Core>

class LinearSVM
{
public:
	LinearSVM(int d, double lambda, int t0 = 1, bool doProject = false, bool useBias = false);
	
	
	double Eval(const Eigen::VectorXd& x);
	void Update(const Eigen::VectorXd& x, int y);
	void Debug();
	
	inline void SetW(const Eigen::VectorXd& w) { m_w = w;}
	inline const Eigen::VectorXd& GetW() const { return m_w; }
	inline double SquaredNorm() const { return m_w.squaredNorm(); }

private:
	int m_d;
	double m_lambda;
	Eigen::VectorXd m_w;
	int m_t;
	bool m_doProject;
	
	bool m_useBias;
	double m_bias;

	void Project();
};

#endif
