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

#include <iostream>
#include <fstream>
#include <vector>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "Config.h"
#include "Detector.h"
#include "Rect.h"
#include "Video.h"
#include "Utils.h"

#include "vot.hpp"

using namespace std;
using namespace cv;

static const int kPatchWidth = 150;
static const int kPatchHeight = 150;
static IntRect rect(320-kPatchWidth/2, 240-kPatchHeight/2, kPatchWidth, kPatchHeight);
static const char* kWindowName = "learnmatch";
static const char* kRecordPath = "D:/data/learnmatch/video";

int main(int argc, char* argv[])
{
	Config conf;
	
	VideoCapture cap;
	Video* pVideoInput = 0;
	Video* pVideoResults = 0;
	vector<Mat> groundTruth;
	Mat frame, frameGrey;
	
	srand(conf.seed);
	
	Detector detector(conf);

	//Check if --challenge was passed as an argument
	bool challengeMode = false;
	for (int i = 1; i < argc; i++) {
		if (strcmp("--challenge", argv[i]) == 0) {
			challengeMode = true;
		}
	}


    if (challengeMode) {

        //load region, images and prepare for output
        VOT vot_io("region.txt", "images.txt", "output.txt");

        //img = firts frame, initPos = initial position in the first frame
        cv::Rect initPos = vot_io.getInitRectangle();
        vot_io.getNextImage(frame);
        cvtColor(frame, frameGrey, CV_RGB2GRAY);

        //output init also bbox
        vot_io.outputBoundingBox(initPos);

        rect = IntRect(initPos.x, initPos.y, initPos.width, initPos.height);

        detector.SetModel(frameGrey, rect);

		while (vot_io.getNextImage(frame) == 1){
			cvtColor(frame, frameGrey, CV_RGB2GRAY);
			
			//Run detector
			detector.Detect(frameGrey, conf.enableLearning);
			
			if (detector.HasDetection())
			{
				vector<Point2f> cornersProj(4);

				double x_min;
				double y_min;
				double x_max;
				double y_max;

				Mat x_values = Mat::zeros(4,1, CV_64F);
				x_values.at<double>(0) = cornersProj[0].x;
				x_values.at<double>(1) = cornersProj[1].x;
				x_values.at<double>(2) = cornersProj[2].x;
				x_values.at<double>(3) = cornersProj[3].x;

				Mat y_values = Mat::zeros(4,1, CV_64F);
				y_values.at<double>(0) = cornersProj[0].y;
				y_values.at<double>(1) = cornersProj[1].y;
				y_values.at<double>(2) = cornersProj[2].y;
				y_values.at<double>(3) = cornersProj[3].y;

				minMaxLoc(x_values, &x_min, &x_max);
				minMaxLoc(y_values, &y_min, &y_max);

				Point2f tl = Point2f(x_min, y_min);
				Point2f br = Point2f(x_max, y_max);

				Rect output = Rect(tl.x, tl.y, br.x - tl.x, br.y - tl.y);

				vot_io.outputBoundingBox(output);

			} else {
				vot_io.p_output_stream << "nan,nan,nan,nan" << std::endl;  
			}
		}
		
		return EXIT_SUCCESS;
    }

	bool useVideo = false;
	bool configRead = false;
	for (int i = 1; i < argc; ++i)
	{
 		string arg(argv[i]);
		if (arg == "--config")
		{
			// read config file
			conf = Config(string(argv[++i]));
			configRead = true;
		}
		else if (arg == "--output")
		{
			// save results video
			pVideoResults = new Video(string(argv[++i]), true);
		}
		else
		{
			cout << "unrecognised option: " << arg << endl;
			return EXIT_FAILURE;
		}
	}
	
	if (!configRead)
	{
		// no config path specified, so use default location
		conf = Config("config.txt");
	}
	
	cout << "config:" << endl;
	cout << conf << endl;

	
	if (conf.useVideo)
	{
		// run from a video
		pVideoInput = new Video(conf.videoPath, false, "%05d.jpg");
		// read groundtruth
		ifstream gtInit((conf.videoPath+"/gt_init.txt").c_str());
		if (gtInit.eof() || gtInit.bad() || gtInit.fail())
		{
			cout << "failed to open gt_init.txt" << endl;
			return EXIT_FAILURE;
		}
		float x, y, w, h;
		gtInit >> x >> y >> w >> h;
		rect = IntRect(x, y, w, h);
		
		ifstream gtHomography((conf.videoPath+"/gt_homography.txt").c_str());
		if (gtInit.eof() || gtInit.bad() || gtInit.fail())
		{
			cout << "failed to open gt_homograhy.txt" << endl;
			return EXIT_FAILURE;
		}
		while (!(gtHomography.eof() || gtHomography.bad() || gtHomography.fail()))
		{
			Mat H(3, 3, CV_64FC1);
			double* p = H.ptr<double>();
			gtHomography >> p[0] >> p[1] >> p[2] >> p[3] >> p[4] >> p[5] >> p[6] >> p[7] >> p[8];
			if (!(gtHomography.eof() || gtHomography.bad() || gtHomography.fail()))
			{
				groundTruth.push_back(H);
			}
		}
		cout << "groundtruth: " << groundTruth.size() << endl;
	}
	
	if (!pVideoInput)
	{
		cap.open(0);
		if (!cap.isOpened())
		{
			cout << "could not open capture device" << endl;
			return EXIT_FAILURE;
		}
		cap.set(CV_CAP_PROP_FPS, 30.0);
		cap.set(CV_CAP_PROP_FRAME_WIDTH , 640.0);
		cap.set(CV_CAP_PROP_FRAME_HEIGHT, 480.0);
	}
	
	Video* pVideoOutput = 0;
	bool doFlip = false;
	if (!conf.quietMode)
	{
		namedWindow(kWindowName);
		cvMoveWindow(kWindowName, 640, 20);
	}
	for (int frameIdx = 0;;++frameIdx)
	{				
		if (!pVideoInput)
		{
			cap.grab();
			cap.retrieve(frame);
			if (doFlip) flip(frame, frame, 1);
			cvtColor(frame, frameGrey, CV_RGB2GRAY);
		}
		else
		{
			bool ok = pVideoInput->ReadFrame(frameGrey);
			if (!ok) break;
			cvtColor(frameGrey, frame, CV_GRAY2RGB);
			DrawHomography(groundTruth[frameIdx], frame, rect.Width(), rect.Height(), CV_RGB(0, 255, 0));
		}

		if (pVideoOutput)
		{
			bool ok = pVideoOutput->WriteFrame(frame);
			if (!ok) cout << "error writing video frame" << endl;
		}
		
		if (pVideoInput && !detector.HasModel())
		{
			detector.SetModel(frameGrey, rect);
		}
		
		if (detector.HasModel())
		{
			detector.Detect(frameGrey, conf.enableLearning);
			
			if (pVideoInput)
			{
				const Mat& Hgt = groundTruth[frameIdx];
				const Mat& H = detector.GetH();
				
				if (detector.HasDetection())
				{
					detector.m_stats.error = HomographyLoss(Hgt, H);
				}
				else
				{
					detector.m_stats.error = 1e8;
				}
				detector.LogStats();
			}
			
			if (detector.HasDetection())
			{
				DrawHomography(detector.GetH(), frame, rect.Width(), rect.Height(), detector.HasDetection() ? CV_RGB(0, 0, 255) : CV_RGB(255, 0, 0));
			}
			
			detector.Debug();
		}
		else
		{
			rectangle(frame, Point2i(rect.XMin(), rect.YMin()), Point2i(rect.XMax(), rect.YMax()), CV_RGB(0, 255, 0), 2);
		}
		
		
		if (pVideoResults)
		{
			bool ok = pVideoResults->WriteFrame(detector.HasModel() ? detector.GetDebugImage() : frame);
			if (!ok) cout << "error writing results video frame" << endl;
		}
		
		if (!conf.quietMode)
		{
			imshow(kWindowName, detector.HasModel() ? detector.GetDebugImage() : frame);
			
			imshow("homography", frame);
			
			int key = waitKey(1);
			if (key != -1)
			{
				if (key == 27 || key == 'q') // 27 = esc
				{
					break;
				}
				else if (key == 'f')
				{
					doFlip = !doFlip;
				}
				else if (key == 'i')
				{
					if (!detector.HasModel())
					{
						detector.SetModel(frameGrey, rect);
					}
					else
					{
						detector.Reset();
					}
				}
				else if (key == 'c')
				{
					detector.m_useClassifiers = !detector.m_useClassifiers;
					cout << "use classifier: " << detector.m_useClassifiers << endl;
				}
				else if (key == 'b')
				{
					detector.m_useBinaryWeightVector = !detector.m_useBinaryWeightVector;
					cout << "use binary weight vector: " << detector.m_useBinaryWeightVector << endl;
				}
				else if (key == 'p')
				{
					waitKey();
				}
				else if (key == 'r' && !pVideoInput)
				{
					if (!pVideoOutput)
					{
						pVideoOutput = new Video(kRecordPath, true);
					}
					else
					{
						delete pVideoOutput;
						pVideoOutput = 0;
					}
				}
			}
		}
	}
	
	if (pVideoInput) delete pVideoInput;
	if (pVideoOutput) delete pVideoOutput;
	if (pVideoResults) delete pVideoResults;
	return EXIT_SUCCESS;
}
