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

#include "RobustHomography.h"

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/core/internal.hpp>

#include <map>

using namespace cv;
using namespace std;

// mostly taken from OpenCV

RobustHomography::RobustHomography(float inlierThreshold, int maxMatches, int maxSamples) :
	PROSAC<Mat>(PROSACParams(maxSamples)),
	m_inlierThreshold2(inlierThreshold*inlierThreshold),
	m_pPointsA(0),
	m_pPointsB(0),
	m_pScores(0),
	m_pModelInds(0),
	m_dataSize(0),
	m_maxMatches(maxMatches)
{
}
	

void RobustHomography::SetDataPointers(const vector<Point2f>* pPointsA, const vector<Point2f>* pPointsB)
{
	m_pPointsA = pPointsA;
	m_pPointsB = pPointsB;
	m_dataSize = m_pPointsA->size();
}

void RobustHomography::SetScores(const vector<double>* pScores)
{
	m_pScores = pScores;
}

void RobustHomography::SetModelInds(const std::vector<int>* pInds)
{
	m_pModelInds = pInds;
}

bool RobustHomography::ValidateSample(const vector<int>& sample)
{
	// check points aren't the same
	for (int i = 0; i < 3; ++i)
	{
		const Point2f& pAi = (*m_pPointsA)[sample[i]];
		const Point2f& pBi = (*m_pPointsB)[sample[i]];
		for (int j = i+1; j < 4; ++j)
		{
			const Point2f& pAj = (*m_pPointsA)[sample[j]];
			const Point2f& pBj = (*m_pPointsB)[sample[j]];
			
			if ((pAi.x == pAj.x && pAi.y == pAj.y) || (pBi.x == pBj.x && pBi.y == pBj.y))
			{
				return false;
			}
		}
	}
	
	
	return true;
}

bool RobustHomography::SolveForHypothesis(const vector<int>& sample, vector<Mat>& rSolutions)
{
	rSolutions.resize(1);
	if (rSolutions[0].empty()) rSolutions[0].create(3, 3, CV_64F);
	CvMat H = rSolutions[0];
	
    int i, count = sample.size();

    double LtL[9][9], W[9][1], V[9][9];
    CvMat _LtL = cvMat( 9, 9, CV_64F, LtL );
    CvMat matW = cvMat( 9, 1, CV_64F, W );
    CvMat matV = cvMat( 9, 9, CV_64F, V );
    CvMat _H0 = cvMat( 3, 3, CV_64F, V[8] );
    CvMat _Htemp = cvMat( 3, 3, CV_64F, V[7] );
    CvPoint2D64f cM={0,0}, cm={0,0}, sM={0,0}, sm={0,0};

    for( i = 0; i < count; i++ )
    {
		Point2f M = (*m_pPointsA)[sample[i]];
		Point2f m = (*m_pPointsB)[sample[i]];
        cm.x += m.x; cm.y += m.y;
        cM.x += M.x; cM.y += M.y;
    }

    cm.x /= count; cm.y /= count;
    cM.x /= count; cM.y /= count;

    for( i = 0; i < count; i++ )
    {
		Point2f M = (*m_pPointsA)[sample[i]];
		Point2f m = (*m_pPointsB)[sample[i]];
        sm.x += fabs(m.x - cm.x);
        sm.y += fabs(m.y - cm.y);
        sM.x += fabs(M.x - cM.x);
        sM.y += fabs(M.y - cM.y);
    }

    if( fabs(sm.x) < DBL_EPSILON || fabs(sm.y) < DBL_EPSILON ||
        fabs(sM.x) < DBL_EPSILON || fabs(sM.y) < DBL_EPSILON )
        return 0;
    sm.x = count/sm.x; sm.y = count/sm.y;
    sM.x = count/sM.x; sM.y = count/sM.y;

    double invHnorm[9] = { 1./sm.x, 0, cm.x, 0, 1./sm.y, cm.y, 0, 0, 1 };
    double Hnorm2[9] = { sM.x, 0, -cM.x*sM.x, 0, sM.y, -cM.y*sM.y, 0, 0, 1 };
    CvMat _invHnorm = cvMat( 3, 3, CV_64FC1, invHnorm );
    CvMat _Hnorm2 = cvMat( 3, 3, CV_64FC1, Hnorm2 );

    cvZero( &_LtL );
    for( i = 0; i < count; i++ )
    {
    	Point2f M = (*m_pPointsA)[sample[i]];
		Point2f m = (*m_pPointsB)[sample[i]];
        double x = (m.x - cm.x)*sm.x, y = (m.y - cm.y)*sm.y;
        double X = (M.x - cM.x)*sM.x, Y = (M.y - cM.y)*sM.y;
        double Lx[] = { X, Y, 1, 0, 0, 0, -x*X, -x*Y, -x };
        double Ly[] = { 0, 0, 0, X, Y, 1, -y*X, -y*Y, -y };
        int j, k;
        for( j = 0; j < 9; j++ )
            for( k = j; k < 9; k++ )
                LtL[j][k] += Lx[j]*Lx[k] + Ly[j]*Ly[k];
    }
    cvCompleteSymm( &_LtL );

    //cvSVD( &_LtL, &matW, 0, &matV, CV_SVD_MODIFY_A + CV_SVD_V_T );
    cvEigenVV( &_LtL, &matV, &matW );
    cvMatMul( &_invHnorm, &_H0, &_Htemp );
    cvMatMul( &_Htemp, &_Hnorm2, &_H0 );
    cvConvertScale( &_H0, &H, 1./_H0.data.db[8] );

    return true;
}

void RobustHomography::ScoreHypothesis(const Mat& hypothesis, float& rScore, vector<unsigned char>& rInliers, int& rNumInliers)
{
    int i, count = m_dataSize;
    CvMat model = hypothesis;
    const double* H = model.data.db;
	double score = 0.0;
	rNumInliers = 0;
	map<int, bool> modelPointUsed;
	
	
	// when m_pScores is set hypotheses are scored by summing the scores of inlier correspondences,
	// otherwise hypotheses are scored according to MLESAC (note this score is negated such that
	// the highest scoring hypothesis would be the MLESAC solution)
	
    for( i = 0; i < count; i++ )
    {
		// ensure each model point is used only once.
		// since the matches are sorted by score, we take
		// only the first (highest scoring) one
		if (m_pModelInds && modelPointUsed.count((*m_pModelInds)[i]))
		{
			rInliers[i] = 0;
			continue;
		}
		Point2f M = (*m_pPointsA)[i];
		Point2f m = (*m_pPointsB)[i];
        double ww = 1./(H[6]*M.x + H[7]*M.y + 1.);
        double dx = (H[0]*M.x + H[1]*M.y + H[2])*ww - m.x;
        double dy = (H[3]*M.x + H[4]*M.y + H[5])*ww - m.y;
        float err = (float)(dx*dx + dy*dy);
        if (err <= m_inlierThreshold2 && !(m_pModelInds && modelPointUsed.count((*m_pModelInds)[i])))
        {
			rInliers[i] = 1;
			++rNumInliers;
			if (m_pScores)
			{
				score += (*m_pScores)[i];
			}
			else
			{
				score -= err;
			}
			if (m_pModelInds)
			{
				modelPointUsed[(*m_pModelInds)[i]] = true;
			}
		}
		else
		{
			rInliers[i] = 0;
			if (!m_pScores)
			{
				score -= m_inlierThreshold2;
			}
		}
    }
	
	rScore = (float)score;
}

bool RobustHomography::Polish()
{
	bool ok = FastPolish();
	if (!ok) return false;
	
	vector<int> sample;
	sample.reserve(m_bestNumInliers);
	for (int i = 0; i < m_dataSize; ++i)
	{
		if (m_bestInliers[i]) sample.push_back(i);
	}
	
	CvMat* model = &(CvMat)m_bestHypothesis;
	int maxIters = 10;
	
    CvLevMarq solver(8, 0, cvTermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, maxIters, DBL_EPSILON));
    int i, j, k, count = sample.size();
    CvMat modelPart = cvMat( solver.param->rows, solver.param->cols, model->type, model->data.ptr );
    cvCopy( &modelPart, solver.param );

    for(;;)
    {
        const CvMat* _param = 0;
        CvMat *_JtJ = 0, *_JtErr = 0;
        double* _errNorm = 0;

        if( !solver.updateAlt( _param, _JtJ, _JtErr, _errNorm ))
            break;

        for( i = 0; i < count; i++ )
        {
            Point2f M = (*m_pPointsA)[sample[i]];
			Point2f m = (*m_pPointsB)[sample[i]];
            const double* h = _param->data.db;
            double Mx = M.x, My = M.y;
            double ww = h[6]*Mx + h[7]*My + 1.;
            ww = fabs(ww) > DBL_EPSILON ? 1./ww : 0;
            double _xi = (h[0]*Mx + h[1]*My + h[2])*ww;
            double _yi = (h[3]*Mx + h[4]*My + h[5])*ww;
            double err[] = { _xi - m.x, _yi - m.y };
            if( _JtJ || _JtErr )
            {
                double J[][8] =
                {
                    { Mx*ww, My*ww, ww, 0, 0, 0, -Mx*ww*_xi, -My*ww*_xi },
                    { 0, 0, 0, Mx*ww, My*ww, ww, -Mx*ww*_yi, -My*ww*_yi }
                };

                for( j = 0; j < 8; j++ )
                {
                    for( k = j; k < 8; k++ )
                        _JtJ->data.db[j*8+k] += J[0][j]*J[0][k] + J[1][j]*J[1][k];
                    _JtErr->data.db[j] += J[0][j]*err[0] + J[1][j]*err[1];
                }
            }
            if( _errNorm )
                *_errNorm += err[0]*err[0] + err[1]*err[1];
        }
    }

    cvCopy( solver.param, &modelPart );
    
    return true;
}
