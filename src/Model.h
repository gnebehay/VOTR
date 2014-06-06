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

#ifndef MODEL_H
#define MODEL_H

#include "Config.h"
#include "BinaryDescriptor.h"
#include "FloatDescriptor.h"
#include "Rect.h"
#include "LinearSVM.h"
#include "RobustHomography.h"
#include "BinaryWeightVector.h"
#include "Stats.h"

#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>

#define BRIEF (0)
#define SURF (1)
#define BRISK (2)

#define DESCRIPTOR_TYPE BRIEF
//#define DESCRIPTOR_TYPE BRISK
//#define DESCRIPTOR_TYPE SURF

#define INDEPENDENT (0)
#define STRUCTURED (1)
#define BOOSTING (2)

#define LEARNER_TYPE STRUCTURED
//#define LEARNER_TYPE INDEPENDENT
//#define LEARNER_TYPE BOOSTING

#define BINARY_DESCRIPTOR ((DESCRIPTOR_TYPE == BRIEF) || (DESCRIPTOR_TYPE == BRISK))

#if LEARNER_TYPE == BOOSTING
#include "StrongClassifierDirectSelection.h"
#endif

class Model
{
public:

#if DESCRIPTOR_TYPE == BRIEF
	static const int kDescriptorLength = 256; 
	typedef BinaryDescriptor<kDescriptorLength> DescriptorType;
#elif DESCRIPTOR_TYPE == BRISK
	static const int kDescriptorLength = 512;
	typedef BinaryDescriptor<kDescriptorLength> DescriptorType;
#elif DESCRIPTOR_TYPE == SURF
	static const int kDescriptorLength = 64;
	typedef FloatDescriptor<kDescriptorLength> DescriptorType;
#endif

	Model(const Config& config, const cv::Mat& image, const IntRect& rect);
	~Model();
	
	bool Detect(const cv::Mat& image, cv::Mat& rH, bool updateModel = false, Stats* pStats = 0);
	bool Update(const cv::Mat& image, const cv::Mat& H);
	
	inline int GetNumKeypoints() const { return m_numKeypoints; }
	inline const cv::Mat& GetImage() const { return m_image; }
	inline const int GetWidth() const { return m_image.cols; }
	inline const int GetHeight() const { return m_image.rows; }
	inline const cv::Mat& GetDebugImage() const { return m_debugImage; }
	
	bool m_useClassifiers;
	bool m_useBinaryWeightVector;
	

private:
	struct Match
	{
		Match() : imageIdx(-1), modelIdx(-1), score(0.f) {}
		
		int imageIdx;
		int modelIdx;
		double score;
		double sortScore;
		int otherImageIdx;
		
		inline bool operator<(const Match& other) const
		{
			//return score > other.score;
			return sortScore > other.sortScore;
		}
	};
	
	const Config& m_config;
	
	cv::FeatureDetector* m_pFeatureDetector;
	cv::DescriptorExtractor* m_pDescriptorExtractor;
	
	int m_numKeypoints;
	std::vector<cv::KeyPoint> m_keypoints;
	std::vector<DescriptorType> m_descriptors;
	Eigen::VectorXd m_w;
	int m_t;
	std::vector<LinearSVM*> m_binaryClassifiers;
	std::vector<BinaryWeightVector<Model::kDescriptorLength>*> m_binaryWeightVectors;
#if LEARNER_TYPE == BOOSTING
	std::vector<OnlineBoosting::StrongClassifierDirectSelection*> m_boostingClassifiers;
#endif

	cv::Mat m_image;
	cv::Mat m_debugImage;

	void ConvertCvDescriptors(const cv::Mat& D, std::vector<DescriptorType>& rDescriptors) const;
	void FindMatches(const std::vector<cv::KeyPoint>& keypoints, const std::vector<DescriptorType>& descriptors, std::vector<Match>& rMatches);
#if LEARNER_TYPE == BOOSTING
	void FindMatchesBoosting(OnlineBoosting::ImageRepresentation* imageRep, const std::vector<cv::KeyPoint>& keypoints, std::vector<Match>& rMatches);
#endif
	void SampleToVector(const PROSACSample<cv::Mat>& s, const std::vector<Match>& matches, const std::vector<DescriptorType>& descriptors, Eigen::VectorXd& rX);
	void UpdateDebugImage(const cv::Mat& image, const std::vector<cv::KeyPoint>& keypoints, const std::vector<Match>& matches, bool detected, const cv::Mat& H, const cv::Mat* Hneg = 0);

};

#endif
