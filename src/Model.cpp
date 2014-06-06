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

#include "Model.h"
#include "Utils.h"
#include "BinaryUtils.h"
#include "Timer.h"

#include "GraphUtils/GraphUtils.h"

#include <vector>
#include <map>
#include <algorithm>

#include <Eigen/Core>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#if DESCRIPTOR_TYPE == BRISK
#include <brisk/brisk.h>
#endif

using namespace std;
using namespace cv;
using namespace Eigen;

Model::Model(const Config& config, const cv::Mat& image, const IntRect& rect) :
	m_config(config),
	m_pFeatureDetector(0),
	m_pDescriptorExtractor(0)
{
	assert(image.type() == CV_8UC1);
	
	m_useClassifiers = true;
	m_useBinaryWeightVector = false;
	
#if DESCRIPTOR_TYPE == BRIEF
	m_pFeatureDetector = new FastFeatureDetector(30);
	m_pDescriptorExtractor = new BriefDescriptorExtractor(kDescriptorLength/8);
#elif DESCRIPTOR_TYPE == BRISK
	m_pFeatureDetector = new BriskFeatureDetector(40);
	m_pDescriptorExtractor = new BriskDescriptorExtractor();
#elif DESCRIPTOR_TYPE == SURF
	m_pFeatureDetector = new SurfFeatureDetector(450.0, 4, 2 );
	m_pDescriptorExtractor = new SurfDescriptorExtractor(4, 2 );
#endif
	
	// use the strongest keypoint locations
	vector<KeyPoint> keypoints;
	m_pFeatureDetector->detect(image, keypoints);
	for (int i = 0; i < keypoints.size(); ++i)
	{
		const KeyPoint& kp = keypoints[i];
		if (rect.ContainsPoint(kp.pt.x, kp.pt.y))
		{
			m_keypoints.push_back(kp);
		}
	}
	sort(m_keypoints.begin(), m_keypoints.end(), CompareKeypoints);


	if (m_keypoints.size() > m_config.maxModelKeypoints)
	{
		m_keypoints.resize(m_config.maxModelKeypoints);
	}
	
	Mat D;
	m_pDescriptorExtractor->compute(image, m_keypoints, D);
	ConvertCvDescriptors(D, m_descriptors);
	
	m_numKeypoints = m_keypoints.size();

#if LEARNER_TYPE == INDEPENDENT
	m_binaryClassifiers.resize(m_numKeypoints);
	m_binaryWeightVectors.resize(m_numKeypoints);
	for (int i = 0; i < m_numKeypoints; ++i)
	{
		m_binaryClassifiers[i] = new LinearSVM(kDescriptorLength, m_config.svmLambda, 10);
		m_binaryClassifiers[i]->SetW(m_descriptors[i].AsVector());
		#if BINARY_DESCRIPTOR		
			m_binaryWeightVectors[i] = new BinaryWeightVector<kDescriptorLength>(m_config.svmBinaryComponents);
			m_binaryWeightVectors[i]->Update(m_binaryClassifiers[i]->GetW());
		#endif
	}
#elif LEARNER_TYPE == STRUCTURED
	m_w = VectorXd::Zero(m_numKeypoints*kDescriptorLength);
	m_t = 10;
	m_binaryWeightVectors.resize(m_numKeypoints);
	for (int i = 0; i < m_numKeypoints; ++i)
	{
		m_w.segment(i*kDescriptorLength, kDescriptorLength) = m_descriptors[i].AsVector();
		#if BINARY_DESCRIPTOR
			m_binaryWeightVectors[i] = new BinaryWeightVector<kDescriptorLength>(m_config.svmBinaryComponents);
			m_binaryWeightVectors[i]->Update(m_w.segment(i*kDescriptorLength, kDescriptorLength));
		#endif
	}
#elif LEARNER_TYPE == BOOSTING
	m_boostingClassifiers.resize(m_numKeypoints);
	OnlineBoosting::ImageRepresentation imageRep((uchar*)image.ptr(), OnlineBoosting::Size(image.rows,image.cols)); 
	for (int i = 0; i < m_numKeypoints; ++i)
	{
		m_boostingClassifiers[i] = new OnlineBoosting::StrongClassifierDirectSelection(20, 150, OnlineBoosting::Size(30,30), true, m_numKeypoints);
		OnlineBoosting::Rect rpos(m_keypoints[i].pt.y-15, m_keypoints[i].pt.x-15, 30, 30);
		for (int j = 0; j < m_numKeypoints; ++j)
		{
			if (j == i) continue;
			OnlineBoosting::Rect rneg(m_keypoints[j].pt.y-15, m_keypoints[j].pt.x-15, 30, 30);
			m_boostingClassifiers[i]->update(&imageRep, rpos, 1);
			m_boostingClassifiers[i]->update(&imageRep, rneg, -1);
		}
	}
#endif

	// this has to be done after descriptor extraction
	for (int i = 0; i < m_numKeypoints; ++i)
	{
		m_keypoints[i].pt.x -= rect.XMin();
		m_keypoints[i].pt.y -= rect.YMin();
	}

	m_image = image(cv::Rect(rect.XMin(), rect.YMin(), rect.Width(), rect.Height())).clone();
	m_debugImage = Mat(image.rows+200, image.cols+rect.Width(), CV_8UC3);
}

Model::~Model()
{
	delete m_pFeatureDetector;
	delete m_pDescriptorExtractor;
	
	for (unsigned int i = 0; i < m_binaryClassifiers.size(); ++i)
	{
		delete m_binaryClassifiers[i];
	}
	for (unsigned int i = 0; i < m_binaryWeightVectors.size(); ++i)
	{
		delete m_binaryWeightVectors[i];
	}
#if LEARNER_TYPE == BOOSTING	
	for (unsigned int i = 0; i < m_boostingClassifiers.size(); ++i)
	{
		delete m_boostingClassifiers[i];
	}
#endif	
}

void Model::SampleToVector(const PROSACSample<Mat>& s, const vector<Match>& matches, const std::vector<DescriptorType>& descriptors, VectorXd& rX)
{
	rX.setZero();
	// note we assume that we can only have one inlier per model keypoint, this is enforced during RANSAC
	for (int i = 0; i < matches.size(); ++i)
	{
		const Match& m = matches[i];
		if (s.inliers[i])
		{
			rX.segment(m.modelIdx*kDescriptorLength, kDescriptorLength) = descriptors[m.imageIdx].AsVector();
		}
	}
}

void Model::ConvertCvDescriptors(const cv::Mat& D, std::vector<DescriptorType>& rDescriptors) const
{
	rDescriptors.reserve(D.rows);
	for (int i = 0; i < D.rows; ++i)
	{
		rDescriptors.push_back(DescriptorType(D.ptr(i)));
	}
}

bool Model::Detect(const cv::Mat& image, cv::Mat& rH, bool updateModel, Stats* pStats)
{
	assert(image.type() == CV_8UC1);

	bool detected = false;
	Timer t;

	vector<KeyPoint> keypoints;
	m_pFeatureDetector->detect(image, keypoints);
	sort(keypoints.begin(), keypoints.end(), CompareKeypoints);
	
	if (keypoints.size() > m_config.maxDetectKeypoints)
	{
		//cout << "limiting keypoints" << endl;
		keypoints.resize(m_config.maxDetectKeypoints);
	}
	
	vector<Match> matches;
#if LEARNER_TYPE == BOOSTING
	vector<KeyPoint> keypointsKept;
	keypointsKept.reserve(keypoints.size());
	for (int i = 0; i < (int)keypoints.size(); ++i)
	{
		const KeyPoint& kp = keypoints[i];
		if (kp.pt.x > 15 && kp.pt.y > 15 && kp.pt.x < image.cols - 16 && kp.pt.y < image.rows - 16)
		{
			keypointsKept.push_back(kp);
		}
	}
	keypoints = keypointsKept;
	
	OnlineBoosting::ImageRepresentation imageRep((uchar*)image.ptr(), OnlineBoosting::Size(image.rows,image.cols)); 
	FindMatchesBoosting(&imageRep, keypoints, matches);
#else
	Mat D;
	m_pDescriptorExtractor->compute(image, keypoints, D);
	vector<DescriptorType> descriptors;
	ConvertCvDescriptors(D, descriptors);
	FindMatches(keypoints, descriptors, matches);
#endif
 
	if (pStats)
	{
		pStats->numKeypoints = keypoints.size();
		pStats->numMatches = matches.size();
	}
	
	Mat HNeg;

	if (matches.size() >= 4)
	{
		// run RANSAC
		std::vector<Point2f> pointsModel;
		std::vector<Point2f> pointsImage;
		std::vector<double> scores;
		std::vector<int> modelInds;
		pointsModel.reserve(matches.size());
		pointsImage.reserve(matches.size());
		scores.reserve(matches.size());
		modelInds.reserve(matches.size());
		for (int i = 0; i < matches.size(); ++i)
		{
			pointsModel.push_back(m_keypoints[matches[i].modelIdx].pt);
			pointsImage.push_back(keypoints[matches[i].imageIdx].pt);
			scores.push_back(matches[i].score);
			modelInds.push_back(matches[i].modelIdx);
		}

		RobustHomography ransac(2.f, m_config.maxMatchesPerModelKeypoint, m_config.prosacIts);
		ransac.SetDataPointers(&pointsModel, &pointsImage);
		ransac.SetModelInds(&modelInds);	
#if LEARNER_TYPE == STRUCTURED
		if (m_useClassifiers)
		{
			ransac.SetScores(&scores);
		}
#endif
		ransac.Compute();
		
		rH = ransac.GetModel().clone();
		int numInliers = ransac.GetNumInliers();
		const vector<unsigned char>& inliers = ransac.GetInliers();

		if (numInliers >= m_numKeypoints/5) // arbitrary threshold...
		{
			detected = true;
			if (ransac.FastPolish() == false) return false;
			rH = ransac.GetModel().clone();

			if (m_useClassifiers && updateModel)
			{
				const vector< PROSACSample<Mat> >& samples = ransac.GetSamples();

				int i = ransac.GetBestSampleIdx();
				
				const PROSACSample<Mat>& si = samples[i];
				
#if LEARNER_TYPE == INDEPENDENT
				
				for (int ii = 0; ii < matches.size(); ++ii)
				{
					// balanced update for matched model points only
					if (si.inliers[ii])
					{
						const Match& mi = matches[ii];
						const DescriptorType& xpos = descriptors[mi.imageIdx];
						for (int jj = 0; jj < matches.size(); ++jj)
						{
							// find the highest-scoring false correspondence for this model keypoint and perform an update.
							// matches are already sorted by score, so this will find the appropriate correspondence.
							if (matches[jj].modelIdx == mi.modelIdx && !si.inliers[jj])
							{
								const DescriptorType& xneg = descriptors[matches[jj].imageIdx];								
								m_binaryClassifiers[mi.modelIdx]->Update(xneg.AsVector(), -1);
								m_binaryClassifiers[mi.modelIdx]->Update(xpos.AsVector(), 1);
								break;
							}
						}					
					}				
				}		
				 
				
				#if BINARY_DESCRIPTOR
					// update binary weight vector approximation
					for (int i = 0; i < m_numKeypoints; ++i)
					{					
						m_binaryWeightVectors[i]->Update(m_binaryClassifiers[i]->GetW());
					}
				#endif

#elif LEARNER_TYPE == STRUCTURED
				int nok = 0;
				int nfail = 0;
				int maxIdx = -1;
				double maxScore = -DBL_MAX;
				double maxLoss = 0.f;
				Mat Hinv;
				if (m_config.lossFunction == Config::kLossFunctionHomography)
				{
					Hinv = si.hypothesis.inv();
				}
				
				for (int j = 0; j < samples.size(); ++j)
				{
					if (j == i) continue;

					const PROSACSample<Mat>& sj = samples[j];

					double loss = 0.0;
					if (m_config.lossFunction == Config::kLossFunctionNumInliers)
					{
						loss = (double)abs(si.numInliers-sj.numInliers);
					}
					else if (m_config.lossFunction == Config::kLossFunctionHomography)
					{
						Mat A = sj.hypothesis * Hinv;
						loss = HomographyLoss(A);
					}
					else if (m_config.lossFunction == Config::kLossFunctionHamming)
					{
						int hamming = 0;
						for (int ii = 0; ii < matches.size(); ++ii)
						{
							hamming += (int)(si.inliers[ii] != sj.inliers[ii]);
						}
						loss = (double)hamming;
					}
					else if (m_config.lossFunction == Config::kLossFunctionNone)
					{
						loss = 0.0;
					}
					//cout << loss << endl;
					
					double diff = si.score - sj.score;

					double score = loss - diff; // margin rescaling
					
					nok += (int)(score < 0.0);
					nfail += (int)(score >= 0.0);
					
					if (score > maxScore)
					{
						maxScore = score;
						maxIdx = j;
						maxLoss = loss;
					}
				}
				
				//cout << "nok: " << nok << " nfail: " << nfail << endl;
				
				m_w *= 1.0 - 1.0/m_t;
				
				if (maxScore <= 1e-5)//0.0)
				{
					// no margin violation					
					cout << "no update" << endl;
				}
				else
				{
					cout << "update: score: " << maxScore << " loss: " << maxLoss << " si: (" << si.score << "," << si.numInliers << ") sj: (" << 
						samples[maxIdx].score << "," << samples[maxIdx].numInliers << ")" << endl;
					HNeg = samples[maxIdx].hypothesis;
					VectorXd xi(m_numKeypoints*kDescriptorLength);
					SampleToVector(si, matches, descriptors, xi);
					VectorXd xj(m_numKeypoints*kDescriptorLength);
					SampleToVector(samples[maxIdx], matches, descriptors, xj);		
					double eta = 1.0/(m_config.svmLambda*m_t);
					m_w += eta*(xi-xj);			
				}
				

				for (int ii = 0; ii < matches.size(); ++ii)
				{
					// balanced update for matched model points only
					if (si.inliers[ii])
					{
						const Match& mi = matches[ii];
						const DescriptorType& xpos = descriptors[mi.imageIdx];
						
						// find the highest-scoring false correspondence for this model keypoint and perform an update.
						// matches are already sorted by score, so this will find the appropriate correspondence.
						for (int jj = 0; jj < matches.size(); ++jj)
						{
							if (matches[jj].modelIdx == mi.modelIdx && !si.inliers[jj])
							{
								const Match& mj = matches[jj];
								const DescriptorType& xneg = descriptors[mj.imageIdx];
								if (mi.score - mj.score < 1.0)
								{
									double eta = m_config.svmNu/(m_config.svmLambda*m_t);
									m_w.segment(mi.modelIdx*kDescriptorLength, kDescriptorLength) += eta*(xpos.AsVector()-xneg.AsVector());
								}
								break;
							}
						}				
					}		
				}
				
				m_t++;
				

				#if BINARY_DESCRIPTOR
					// update binary weight vector approximation
					for (int i = 0; i < m_numKeypoints; ++i)
					{
						m_binaryWeightVectors[i]->Update(m_w.segment(i*kDescriptorLength, kDescriptorLength));
					}
				#endif

#elif LEARNER_TYPE == BOOSTING
				// balanced update
				map<int,bool> updated;
				for (int ii = 0; ii < matches.size(); ++ii)
				{
					// balanced update for matched model points only
					if (si.inliers[ii])
					{
						const Match& mi = matches[ii];
						OnlineBoosting::Rect rpos(keypoints[mi.imageIdx].pt.y-15, keypoints[mi.imageIdx].pt.x-15, 30, 30);
						for (int jj = 0; jj < matches.size(); ++jj)
						{
							if (matches[jj].modelIdx == mi.modelIdx && !si.inliers[jj])
							{								
								OnlineBoosting::Rect rneg(keypoints[matches[jj].imageIdx].pt.y-15, keypoints[matches[jj].imageIdx].pt.x-15, 30, 30);
								
								m_boostingClassifiers[mi.modelIdx]->update(&imageRep, rpos, 1);
								m_boostingClassifiers[mi.modelIdx]->update(&imageRep, rneg, -1);
								
								updated[mi.modelIdx] = true;
							}
						}
					}				
				}
#endif
			}
		}
		
		if (pStats)
		{
			pStats->hasDetection = detected;
			pStats->numInliers = numInliers;
			pStats->bestSampleIndex = ransac.GetBestSampleIdx();
		}
		
		cout << numInliers << " inliers" << endl;
	}
	
	if (pStats)
	{
		pStats->timeDetect = t.GetSeconds();
	}
	
	UpdateDebugImage(image, keypoints, matches, detected, rH, HNeg.empty() ? 0 : &HNeg);

	return detected;
}

bool Model::Update(const cv::Mat& image, const cv::Mat& gtH)
{
	assert(image.type() == CV_8UC1);
	return true;
}

void Model::UpdateDebugImage(const Mat& image, const vector<KeyPoint>& keypoints, const vector<Match>& matches, bool detected, const Mat& H, const Mat* Hneg)
{
	m_debugImage.setTo(0);
	Mat F = m_debugImage(cv::Rect(GetWidth(), 0, image.cols, image.rows));
	drawKeypoints(image, keypoints, F, CV_RGB(0, 0, 255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	if (detected)
	{
		DrawHomography(H, F, GetWidth(), GetHeight(), CV_RGB(0, 255, 0));
		if (Hneg)
		{
			DrawHomography(*Hneg, F, GetWidth(), GetHeight(), CV_RGB(255, 0, 0));
		}
		
	}
	Mat M = m_debugImage(cv::Rect(0, 0, m_image.cols, m_image.rows));
	drawKeypoints(m_image, m_keypoints, M, CV_RGB(0, 0, 255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	
	vector<float> vals(m_numKeypoints);
	vector<unsigned char> topVals(m_numKeypoints);
	double maxW = -DBL_MAX;
	double minW = DBL_MAX;
	for (int i = 0; i < m_numKeypoints; ++i)
	{
		double w = 0.0;
#if LEARNER_TYPE == STRUCTURED		
		w = m_w.segment(i*Model::kDescriptorLength, Model::kDescriptorLength).squaredNorm();
#elif LEARNER_TYPE == INDEPENDENT
		w = m_binaryClassifiers[i]->GetW().squaredNorm();
#endif	
		vals[i] = (float)w;
		maxW = std::max(maxW, w);
		minW = std::min(minW, w);
	}	
	
	double maxScore = -DBL_MAX;
	double minScore = DBL_MAX;
	for (int i = 0; i < matches.size(); ++i)
	{
		maxScore = std::max(maxScore, matches[i].score);
		minScore = std::min(minScore, matches[i].score);
	}
	for (int i = matches.size()-1; i >= 0; --i)
	{
		float s;
		//s = (matches[i].score-minScore)/(maxScore-minScore);
		if (minScore >= 0.f)
			s = (float)(matches[i].score-minScore)/(maxScore-minScore);
		else
			s = (float)(matches[i].score)/std::max(maxScore,-minScore);
		line(m_debugImage,m_keypoints[matches[i].modelIdx].pt,
			Point2f(keypoints[matches[i].imageIdx].pt.x+M.cols, keypoints[matches[i].imageIdx].pt.y),
			CV_RGB(fabs(s)*255, std::max(s,0.0f)*255, 0));
	}
	
	Mat I = m_debugImage(cv::Rect(0, image.rows, m_debugImage.cols, 200));
	I.setTo(cv::Scalar(255,255,255));
	IplImage II = I;
	setGraphColor(0);
	drawFloatGraph(&vals[0], vals.size(), &II, 0.f, (float)maxW, I.cols, I.rows);	
	
	char buf[100];
	sprintf(buf, "Model");
	putText(m_debugImage, buf, Point(15,M.rows + 20), FONT_HERSHEY_DUPLEX, 0.5, CV_RGB(255,255,255), 1, CV_AA);
	putText(m_debugImage, "Matching using:", Point(15,M.rows + 100), FONT_HERSHEY_DUPLEX, 0.5, CV_RGB(255,255,255), 1, CV_AA);
	if (m_useClassifiers)
		sprintf(buf, "Learned weights");
	else
	{
#if DESCRIPTOR_TYPE == BRIEF	
		sprintf(buf, "BRIEF descriptors");
#elif DESCRIPTOR_TYPE == BRISK
		sprintf(buf, "BRISK descriptors");
#elif DESCRIPTOR_TYPE == SURF
		sprintf(buf, "SURF descriptors");
#endif
	}		
	putText(m_debugImage, buf, Point(15,M.rows + 115), FONT_HERSHEY_DUPLEX, 0.5, CV_RGB(255,255,0), 1, CV_AA);
	if (m_useClassifiers)
	{
		putText(m_debugImage, "Binary approximation:", Point(15,M.rows + 150), FONT_HERSHEY_DUPLEX, 0.5, CV_RGB(255,255,255), 1, CV_AA);
		sprintf(buf, "%s", m_useBinaryWeightVector ? "On" : "Off");
		putText(m_debugImage, buf, Point(15,M.rows + 165), FONT_HERSHEY_DUPLEX, 0.5, CV_RGB(255,255,0), 1, CV_AA);
	}

	sprintf(buf, "Norm of weight vector per model keypoint (min:%.8f max:%.8f)", minW, maxW);
	putText(m_debugImage, buf, Point(15,m_debugImage.rows-20), FONT_HERSHEY_DUPLEX, 0.5, CV_RGB(0,0,0), 1, CV_AA);
}

void Model::FindMatches(const vector<KeyPoint>& keypoints, const vector<DescriptorType>& descriptors, vector<Match>& rMatches)
{
	rMatches.reserve(m_numKeypoints*m_config.maxMatchesPerModelKeypoint);
	
	vector<Match> modelMatches(keypoints.size());
	for (int iModel = 0; iModel < m_numKeypoints; ++iModel)
	{
		for (int iImage = 0; iImage < keypoints.size(); ++iImage)
		{
			double score;
			if (m_useClassifiers)
			{
#if LEARNER_TYPE == INDEPENDENT
				#if BINARY_DESCRIPTOR
					if (m_useBinaryWeightVector)
					{
						score = m_binaryWeightVectors[iModel]->Dot(descriptors[iImage]);
					}
					else
					{
						score = m_binaryClassifiers[iModel]->Eval(descriptors[iImage].AsVector());
					}
				#else
					score = m_binaryClassifiers[iModel]->Eval(descriptors[iImage].AsVector());
				#endif
#elif LEARNER_TYPE == STRUCTURED
				#if BINARY_DESCRIPTOR
					if (m_useBinaryWeightVector)
					{
						score = m_binaryWeightVectors[iModel]->Dot(descriptors[iImage]);
					}
					else
					{
						score = m_w.segment(iModel*kDescriptorLength, kDescriptorLength).dot(descriptors[iImage].AsVector());
					}
				#else
					score = m_w.segment(iModel*kDescriptorLength, kDescriptorLength).dot(descriptors[iImage].AsVector());
				#endif
#endif
			}
			else
			{
				#if BINARY_DESCRIPTOR			
					unsigned int d = HammingDistance(m_descriptors[iModel], descriptors[iImage]);
				#else			
					double d = (m_descriptors[iModel].AsVector()-descriptors[iImage].AsVector()).norm();
				#endif				
				score = 1.0 - ((double)d)/kDescriptorLength;
			}
			
			modelMatches[iImage].imageIdx = iImage;
			modelMatches[iImage].modelIdx = iModel;
			modelMatches[iImage].score = score;
			modelMatches[iImage].sortScore = score;
		}
		
		// find the top n matches
		int n = min(m_config.maxMatchesPerModelKeypoint, (int)modelMatches.size());
		
		partial_sort(modelMatches.begin(), modelMatches.begin()+n, modelMatches.end());
		
		rMatches.insert(rMatches.end(), modelMatches.begin(), modelMatches.begin()+n);	
	}
	
	sort(rMatches.begin(), rMatches.end());
}

#if LEARNER_TYPE == BOOSTING
void Model::FindMatchesBoosting(OnlineBoosting::ImageRepresentation* imageRep, const std::vector<cv::KeyPoint>& keypoints, std::vector<Match>& rMatches)
{
	rMatches.reserve(m_numKeypoints*m_config.maxMatchesPerModelKeypoint);
	
	vector<Match> modelMatches(keypoints.size());
	
	for (int iModel = 0; iModel < m_numKeypoints; ++iModel)
	{
		double bestScore = -DBL_MAX;
		int bestIdx = -1;
		int npos = 0;
		int nneg = 0;
		
		for (int iImage = 0; iImage < keypoints.size(); ++iImage)
		{
			OnlineBoosting::Rect r(keypoints[iImage].pt.y-15, keypoints[iImage].pt.x-15, 30, 30);
			double score = m_boostingClassifiers[iModel]->eval(imageRep, r);

			if (score > bestScore)
			{
				bestScore = score;
				bestIdx = iImage;
			}
			
			modelMatches[iImage].imageIdx = iImage;
			modelMatches[iImage].modelIdx = iModel;
			modelMatches[iImage].score = score;
			modelMatches[iImage].sortScore = score;
		}
		
		// top n matches
		int n = min(m_config.maxMatchesPerModelKeypoint, (int)modelMatches.size());
		partial_sort(modelMatches.begin(), modelMatches.begin()+n, modelMatches.end());
		
		rMatches.insert(rMatches.end(), modelMatches.begin(), modelMatches.begin()+n);
		
	}
	
	sort(rMatches.begin(), rMatches.end());
}
#endif
