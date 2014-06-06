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

#include "LinearSVM.h"

#include <iostream>
#include <cmath>

using namespace std;
using namespace Eigen;

LinearSVM::LinearSVM(int d, double lambda, int t0, bool doProject, bool useBias) :
	m_d(d),
	m_lambda(lambda),
	m_t(t0),
	m_doProject(doProject),
	m_useBias(useBias)
{
	m_w = VectorXd::Zero(m_d);
	m_bias = 0.0;
	if (m_doProject) Project();
}

double LinearSVM::Eval(const VectorXd& x)
{
	return m_w.dot(x) + m_bias;
}

void LinearSVM::Update(const VectorXd& x, int y)
{
	double m = 1.0 - y*Eval(x);

	m_w *= 1.0 - 1.0/m_t;
	if (m_useBias) m_bias *= 1.0 - 1.0/m_t;
	
	if (m > 0.0)
	{
		double eta = 1.0/(m_lambda*m_t);
		m_w += eta*y*x;
		if (m_useBias) m_bias += eta*y;
	}
	
	if (m_doProject) Project();

	m_t++;
}

void LinearSVM::Project()
{
	double proj = 1.0/sqrt(m_lambda*m_w.squaredNorm());
	if (proj < 1.0)
	{
		cout << "project (" << proj << ")" << endl;
		m_w *= proj;
	}
}

void LinearSVM::Debug()
{
	cout << "w: " << m_w.transpose() << endl;
}
