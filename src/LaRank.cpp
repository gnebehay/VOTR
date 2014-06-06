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

#include "LaRank.h"

#include "Config.h"
#include "Features.h"
#include "Kernels.h"
#include "Sample.h"
#include "Rect.h"
#include "GraphUtils/GraphUtils.h"

#include <Eigen/Array>

#include <opencv/highgui.h>
static const int kTileSize = 30;
using namespace cv;

using namespace std;
using namespace Eigen;

static const int kMaxSVs = 2000; // TODO (only used when no budget)


LaRank::LaRank(const Config& conf, const Features& features, const Kernel& kernel) :
	m_config(conf),
	m_features(features),
	m_kernel(kernel),
	m_C(conf.svmC)
{
	int N = conf.svmBudgetSize > 0 ? conf.svmBudgetSize+2 : kMaxSVs;
	m_K = MatrixXd::Zero(N, N);
	m_debugImage = Mat(800, 600, CV_8UC3);
}

LaRank::~LaRank()
{
}

double LaRank::Evaluate(const Eigen::VectorXd& x, const FloatRect& y) const
{
	double f = 0.0;
	for (int i = 0; i < (int)m_svs.size(); ++i)
	{
		const SupportVector& sv = *m_svs[i];
		f += sv.b*m_kernel.Eval(x, sv.x->x[sv.y]);
	}
	return f;
}

void LaRank::Eval(const MultiSample& sample, std::vector<double>& results)
{
	const FloatRect& centre(sample.GetRects()[0]);
	vector<VectorXd> fvs;
	const_cast<Features&>(m_features).Eval(sample, fvs);
	results.resize(fvs.size());
	for (int i = 0; i < (int)fvs.size(); ++i)
	{
		// express y in coord frame of centre sample
		FloatRect y(sample.GetRects()[i]);
		y.Translate(-centre.XMin(), -centre.YMin());
		results[i] = Evaluate(fvs[i], y);
	}
}

void LaRank::Update(const MultiSample& sample, int y)
{
	// add new support pattern
	SupportPattern* sp = new SupportPattern;
	const vector<FloatRect>& rects = sample.GetRects();
	FloatRect centre = rects[y];
	for (int i = 0; i < (int)rects.size(); ++i)
	{
		// express r in coord frame of centre sample
		FloatRect r = rects[i];
		r.Translate(-centre.XMin(), -centre.YMin());
		sp->yv.push_back(r);
		if (!m_config.quietMode && m_config.debugMode)
		{
			// store a thumbnail for each sample
			Mat im(kTileSize, kTileSize, CV_8UC1);
			IntRect rect = rects[i];
			cv::Rect roi(rect.XMin(), rect.YMin(), rect.Width(), rect.Height());
			cv::resize(sample.GetImage().GetImage(0)(roi), im, im.size());
			sp->images.push_back(im);
		}
	}
	// evaluate features for each sample
	sp->x.resize(rects.size());
	const_cast<Features&>(m_features).Eval(sample, sp->x);
	sp->y = y;
	sp->refCount = 0;
	m_sps.push_back(sp);

	ProcessNew((int)m_sps.size()-1);
	BudgetMaintenance();
	
	for (int i = 0; i < 10; ++i)
	{
		Reprocess();
		BudgetMaintenance();
	}
}

void LaRank::BudgetMaintenance()
{
	if (m_config.svmBudgetSize > 0)
	{
		while ((int)m_svs.size() > m_config.svmBudgetSize)
		{
			BudgetMaintenanceRemove();
		}
	}
}

void LaRank::Reprocess()
{
	ProcessOld();
	for (int i = 0; i < 10; ++i)
	{
		Optimize();
	}
}

double LaRank::ComputeDual() const
{
	double d = 0.0;
	for (int i = 0; i < (int)m_svs.size(); ++i)
	{
		const SupportVector* sv = m_svs[i];
		d -= sv->b*Loss(sv->x->yv[sv->y], sv->x->yv[sv->x->y]);
		for (int j = 0; j < (int)m_svs.size(); ++j)
		{
			d -= 0.5*sv->b*m_svs[j]->b*m_K(i,j);
		}
	}
	return d;
}

void LaRank::SMOStep(int ipos, int ineg)
{
	if (ipos == ineg) return;

	SupportVector* svp = m_svs[ipos];
	SupportVector* svn = m_svs[ineg];
	assert(svp->x == svn->x);
	SupportPattern* sp = svp->x;

#if VERBOSE
	cout << "SMO: gpos:" << svp->g << " gneg:" << svn->g << endl;
#endif	
	if ((svp->g - svn->g) < 1e-5)
	{
#if VERBOSE
		cout << "SMO: skipping" << endl;
#endif		
	}
	else
	{
		double kii = m_K(ipos, ipos) + m_K(ineg, ineg) - 2*m_K(ipos, ineg);
		double lu = (svp->g-svn->g)/kii;
		// no need to clamp against 0 since we'd have skipped in that case
		double l = min(lu, m_C*(int)(svp->y == sp->y) - svp->b);

		svp->b += l;
		svn->b -= l;

		// update gradients
		for (int i = 0; i < (int)m_svs.size(); ++i)
		{
			SupportVector* svi = m_svs[i];
			svi->g -= l*(m_K(i, ipos) - m_K(i, ineg));
		}
#if VERBOSE
		cout << "SMO: " << ipos << "," << ineg << " -- " << svp->b << "," << svn->b << " (" << l << ")" << endl;
#endif		
	}
	
	// check if we should remove either sv now
	
	if (fabs(svp->b) < 1e-8)
	{
		RemoveSupportVector(ipos);
		if (ineg == (int)m_svs.size())
		{
			// ineg and ipos will have been swapped during sv removal
			ineg = ipos;
		}
	}

	if (fabs(svn->b) < 1e-8)
	{
		RemoveSupportVector(ineg);
	}
}

pair<int, double> LaRank::MinGradient(int ind)
{
	const SupportPattern* sp = m_sps[ind];
	pair<int, double> minGrad(-1, DBL_MAX);
	for (int i = 0; i < (int)sp->yv.size(); ++i)
	{
		double grad = -Loss(sp->yv[i], sp->yv[sp->y]) - Evaluate(sp->x[i], sp->yv[i]);
		if (grad < minGrad.second)
		{
			minGrad.first = i;
			minGrad.second = grad;
		}
	}
	return minGrad;
}

void LaRank::ProcessNew(int ind)
{
	// gradient is -f(x,y) since loss=0
	int ip = AddSupportVector(m_sps[ind], m_sps[ind]->y, -Evaluate(m_sps[ind]->x[m_sps[ind]->y],m_sps[ind]->yv[m_sps[ind]->y]));

	pair<int, double> minGrad = MinGradient(ind);
	int in = AddSupportVector(m_sps[ind], minGrad.first, minGrad.second);

	SMOStep(ip, in);
}

void LaRank::ProcessOld()
{
	if (m_sps.size() == 0) return;

	// choose pattern to process
	int ind = rand() % m_sps.size();

	// find existing sv with largest grad and nonzero beta
	int ip = -1;
	double maxGrad = -DBL_MAX;
	for (int i = 0; i < (int)m_svs.size(); ++i)
	{
		if (m_svs[i]->x != m_sps[ind]) continue;

		const SupportVector* svi = m_svs[i];
		if (svi->g > maxGrad && svi->b < m_C*(int)(svi->y == m_sps[ind]->y))
		{
			ip = i;
			maxGrad = svi->g;
		}
	}
	assert(ip != -1);
	if (ip == -1) return;

	// find potentially new sv with smallest grad
	pair<int, double> minGrad = MinGradient(ind);
	int in = -1;
	for (int i = 0; i < (int)m_svs.size(); ++i)
	{
		if (m_svs[i]->x != m_sps[ind]) continue;

		if (m_svs[i]->y == minGrad.first)
		{
			in = i;
			break;
		}
	}
	if (in == -1)
	{
		// add new sv
		in = AddSupportVector(m_sps[ind], minGrad.first, minGrad.second);
	}

	SMOStep(ip, in);
}

void LaRank::Optimize()
{
	if (m_sps.size() == 0) return;
	
	// choose pattern to optimize
	int ind = rand() % m_sps.size();

	int ip = -1;
	int in = -1;
	double maxGrad = -DBL_MAX;
	double minGrad = DBL_MAX;
	for (int i = 0; i < (int)m_svs.size(); ++i)
	{
		if (m_svs[i]->x != m_sps[ind]) continue;

		const SupportVector* svi = m_svs[i];
		if (svi->g > maxGrad && svi->b < m_C*(int)(svi->y == m_sps[ind]->y))
		{
			ip = i;
			maxGrad = svi->g;
		}
		if (svi->g < minGrad)
		{
			in = i;
			minGrad = svi->g;
		}
	}
	assert(ip != -1 && in != -1);
	if (ip == -1 || in == -1)
	{
		// this shouldn't happen
		cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
		return;
	}

	SMOStep(ip, in);
}

int LaRank::AddSupportVector(SupportPattern* x, int y, double g)
{
	SupportVector* sv = new SupportVector;
	sv->b = 0.0;
	sv->x = x;
	sv->y = y;
	sv->g = g;

	int ind = (int)m_svs.size();
	m_svs.push_back(sv);
	x->refCount++;

#if VERBOSE
	cout << "Adding SV: " << ind << endl;
#endif

	// update kernel matrix
	for (int i = 0; i < ind; ++i)
	{
		m_K(i,ind) = m_kernel.Eval(m_svs[i]->x->x[m_svs[i]->y], x->x[y]);
		m_K(ind,i) = m_K(i,ind);
	}
	m_K(ind,ind) = m_kernel.Eval(x->x[y]);

	return ind;
}

void LaRank::SwapSupportVectors(int ind1, int ind2)
{
	SupportVector* tmp = m_svs[ind1];
	m_svs[ind1] = m_svs[ind2];
	m_svs[ind2] = tmp;
	
	VectorXd row1 = m_K.row(ind1);
	m_K.row(ind1) = m_K.row(ind2);
	m_K.row(ind2) = row1;
	
	VectorXd col1 = m_K.col(ind1);
	m_K.col(ind1) = m_K.col(ind2);
	m_K.col(ind2) = col1;
}

void LaRank::RemoveSupportVector(int ind)
{
#if VERBOSE
	cout << "Removing SV: " << ind << endl;
#endif	

	m_svs[ind]->x->refCount--;
	if (m_svs[ind]->x->refCount == 0)
	{
		// also remove the support pattern
		for (int i = 0; i < (int)m_sps.size(); ++i)
		{
			if (m_sps[i] == m_svs[ind]->x)
			{
				delete m_sps[i];
				m_sps.erase(m_sps.begin()+i);
				break;
			}
		}
	}

	// make sure the support vector is at the back, this
	// lets us keep the kernel matrix cached and valid
	if (ind < (int)m_svs.size()-1)
	{
		SwapSupportVectors(ind, (int)m_svs.size()-1);
		ind = (int)m_svs.size()-1;
	}
	delete m_svs[ind];
	m_svs.pop_back();
}

void LaRank::BudgetMaintenanceRemove()
{
	// find negative sv with smallest effect on discriminant function if removed
	double minVal = DBL_MAX;
	int in = -1;
	int ip = -1;
	for (int i = 0; i < (int)m_svs.size(); ++i)
	{
		if (m_svs[i]->b < 0.0)
		{
			// find corresponding positive sv
			int j = -1;
			for (int k = 0; k < (int)m_svs.size(); ++k)
			{
				if (m_svs[k]->b > 0.0 && m_svs[k]->x == m_svs[i]->x)
				{
					j = k;
					break;
				}
			}
			double val = m_svs[i]->b*m_svs[i]->b*(m_K(i,i) + m_K(j,j) - 2.0*m_K(i,j));
			if (val < minVal)
			{
				minVal = val;
				in = i;
				ip = j;
			}
		}
	}

	// adjust weight of positive sv to compensate for removal of negative
	m_svs[ip]->b += m_svs[in]->b;

	// remove negative sv
	RemoveSupportVector(in);
	if (ip == (int)m_svs.size())
	{
		// ip and in will have been swapped during support vector removal
		ip = in;
	}
	
	if (m_svs[ip]->b < 1e-8)
	{
		// also remove positive sv
		RemoveSupportVector(ip);
	}

	// update gradients
	// TODO: this could be made cheaper by just adjusting incrementally rather than recomputing
	for (int i = 0; i < (int)m_svs.size(); ++i)
	{
		SupportVector& svi = *m_svs[i];
		svi.g = -Loss(svi.x->yv[svi.y],svi.x->yv[svi.x->y]) - Evaluate(svi.x->x[svi.y], svi.x->yv[svi.y]);
	}	
}

void LaRank::Debug()
{
	cout << m_sps.size() << "/" << m_svs.size() << " support patterns/vectors" << endl;
	UpdateDebugImage();
	imshow("learner", m_debugImage);
}

void LaRank::UpdateDebugImage()
{
	m_debugImage.setTo(0);
	
	int n = (int)m_svs.size();
	
	if (n == 0) return;
	
	const int kCanvasSize = 600;
	int gridSize = (int)sqrtf((float)(n-1)) + 1;
	int tileSize = (int)((float)kCanvasSize/gridSize);
	
	if (tileSize < 5)
	{
		cout << "too many support vectors to display" << endl;
		return;
	}
	
	Mat temp(tileSize, tileSize, CV_8UC1);
	int x = 0;
	int y = 0;
	int ind = 0;
	float vals[kMaxSVs];
	memset(vals, 0, sizeof(float)*n);
	int drawOrder[kMaxSVs];
	
	for (int set = 0; set < 2; ++set)
	{
		for (int i = 0; i < n; ++i)
		{
			if (((set == 0) ? 1 : -1)*m_svs[i]->b < 0.0) continue;
			
			drawOrder[ind] = i;
			vals[ind] = (float)m_svs[i]->b;
			++ind;
			
			Mat I = m_debugImage(cv::Rect(x, y, tileSize, tileSize));
			resize(m_svs[i]->x->images[m_svs[i]->y], temp, temp.size());
			cvtColor(temp, I, CV_GRAY2RGB);
			double w = 1.0;
			rectangle(I, Point(0, 0), Point(tileSize-1, tileSize-1), (m_svs[i]->b > 0.0) ? CV_RGB(0, (uchar)(255*w), 0) : CV_RGB((uchar)(255*w), 0, 0), 3);
			x += tileSize;
			if ((x+tileSize) > kCanvasSize)
			{
				y += tileSize;
				x = 0;
			}
		}
	}
	
	const int kKernelPixelSize = 2;
	int kernelSize = kKernelPixelSize*n;
	
	double kmin = m_K.minCoeff();
	double kmax = m_K.maxCoeff();
	
	if (kernelSize < m_debugImage.cols && kernelSize < m_debugImage.rows)
	{
		Mat K = m_debugImage(cv::Rect(m_debugImage.cols-kernelSize, m_debugImage.rows-kernelSize, kernelSize, kernelSize));
		for (int i = 0; i < n; ++i)
		{
			for (int j = 0; j < n; ++j)
			{
				Mat Kij = K(cv::Rect(j*kKernelPixelSize, i*kKernelPixelSize, kKernelPixelSize, kKernelPixelSize));
				uchar v = (uchar)(255*(m_K(drawOrder[i], drawOrder[j])-kmin)/(kmax-kmin));
				Kij.setTo(Scalar(v, v, v));
			}
		}
	}
	else
	{
		kernelSize = 0;
	}
	
	
	Mat I = m_debugImage(cv::Rect(0, m_debugImage.rows - 200, m_debugImage.cols-kernelSize, 200));
	I.setTo(Scalar(255,255,255));
	IplImage II = I;
	setGraphColor(0);
	drawFloatGraph(vals, n, &II, 0.f, 0.f, I.cols, I.rows);
}
