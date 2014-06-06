/******************************************************************************
 * Author: Martin Godec
 *         godec@icg.tugraz.at
 ******************************************************************************/

#include "parameters.h"
#include "capture.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "cv.h"
#include "cxcore.h"
#include "cvaux.h"
#include "highgui.h"

#include "features.h"
#include "fern.h"
#include "utilities.h"

#include <sys/stat.h>

#define STEP_WIDTH 1
#define SHIFT_TO_CENTER
#define SEARCH_WINDOW 20
#define GRABCUT_ROUNDS 3

using namespace std;
using namespace cv;

void update(Ferns& ferns, Features& ft, const Rect& ROI, const Rect& object, const Point& center, Mat& mask);
void showResult(Mat& frame, Mat& backprojected, const Rect& ROI, const Rect& object, int frameNr, std::string seq_name);
void showSegmentation(Mat& backproject, std::string title);
Point centerOfMass(Mat& mask);
inline Rect squarify(Rect object, double searchFactor);
Rect getBoundingBox(Mat& backproject);
void makeBinarySegmentation(Mat& backproject);

#include "vot.hpp"

void run_challenge() {
	Mat frame, frameGrey;
	//load region, images and prepare for output
	VOT vot_io("region.txt", "images.txt", "output.txt");
	Rect object = vot_io.getInitRectangle();
	vot_io.getNextImage(frame);
	//output init also bbox
	vot_io.outputBoundingBox(object);

	float maxScale = 1.0;
	float backProjectRadius = 0.5;
	float backProjectminProb = 0.5;

	Rect max_object;
	Size imgSize = Size(frame.cols, frame.rows);
	int baseSize = 12;
	Rect imgRect = Rect(baseSize/2, baseSize/2, imgSize.width-baseSize, imgSize.height-baseSize);
	std::vector<Rect> trackedPositions;
	
	Features ft;
	ft.setImage(frame);
	Mat result(imgSize.height, imgSize.width, CV_32FC1, Scalar(0.0f));

	Mat backproject( imgSize.height, imgSize.width, CV_8UC1, Scalar(GC_BGD) );
	rectangle(backproject, cvPoint(object.x-10, object.y-10), cvPoint(object.x+object.width+10, object.y+object.height+10), Scalar(GC_PR_BGD), -1);
	rectangle(backproject, cvPoint(object.x, object.y), cvPoint(object.x+object.width, object.y+object.height), Scalar(GC_FGD), -1);

	Ferns ferns(20, Size(baseSize, baseSize), 8, ft.getNumChannels());
	
	max_object = intersect( imgRect, squarify(object, maxScale));
	double minVal, maxVal = 6.0f;
	Point minLoc, maxLoc;
	Point translation;
	Point center = getCenter(object);
	maxLoc = center;

	std::cout << " INITIAL POSITION: " << object.x << "/" << object.y << " " << object.width << "x" << object.height << std::endl;

	Rect updateRegion = intersect(max_object + Size(40,40) - Point(20,20), imgRect);

	std::cout << " UPDATE Ferns initially: ";
	update(ferns, ft, updateRegion, object, center, backproject);
	trackedPositions.push_back(object);

	int frameNr = 0;
	Rect searchWindow = max_object + Size(SEARCH_WINDOW,SEARCH_WINDOW) - Point(SEARCH_WINDOW/2,SEARCH_WINDOW/2);
	
	clock_t ft_clocks = 0;
	clock_t det_clocks = 0;
	clock_t bp_clocks = 0;
	clock_t seg_clocks = 0;
	clock_t up_clocks = 0;
	
	clock_t clocks_start;

	std::cout << " START TRACKING" << std::endl;
	
	while (vot_io.getNextImage(frame) == 1){
		std::cout << std::endl << "-- LOAD FRAME [" << frameNr << "] ------------------------------------------" << std::endl;

		clocks_start = clock();
		ft.setImage(frame);
		ft_clocks += (clock() - clocks_start);
		result = Scalar(0.0);

		std::cout << " EVALUATE" << std::endl;

		clocks_start = clock();
		ferns.evaluate(ft, intersect(searchWindow, imgRect), result, STEP_WIDTH, 0.5f);
		det_clocks += (clock() - clocks_start);
		
		imshow("Result", result);
		Mat out = result;
		normalize(out, out, 255, 0, NORM_MINMAX);
		
		minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);
		std::cout << " LOCATE: maximum is at (" << maxLoc.x << "/" << maxLoc.y << ": " << maxVal << " )" << std::endl;

		if(maxVal < 3.0f)
		{
			trackedPositions.push_back(object);
			continue;
		}

		translation = Point(maxLoc.x - center.x, maxLoc.y - center.y);
		center = Point(maxLoc.x, maxLoc.y);

		setCenter(max_object, center);
		setCenter(object, center);
		
		setCenter(searchWindow, center);
		
		std::cout << " BACKPROJECT" << std::endl;

		backproject = Scalar(GC_BGD);
		rectangle(backproject, cvPoint(max_object.x, max_object.y), cvPoint(max_object.x+max_object.width, max_object.y+max_object.height), Scalar(GC_PR_BGD), -1);

		clocks_start = clock();
		int cnt = ferns.backProject(ft, backproject, intersect( max_object, imgRect), maxLoc, backProjectRadius, STEP_WIDTH, backProjectminProb);
		bp_clocks += (clock() - clocks_start);

		if(cnt > 0)
		{
			std::cout << " SEGMENT" << std::endl;
			Mat subframe(frame, intersect( searchWindow, imgRect));
			Mat subbackproject(backproject, intersect( searchWindow, imgRect));
			clocks_start = clock();
			Mat fgmdl, bgmdl;
			grabCut(subframe, subbackproject, object, fgmdl, bgmdl, GRABCUT_ROUNDS, GC_INIT_WITH_MASK);
			seg_clocks += (clock() - clocks_start);
		
			std::cout << " UPDATE: ";

#ifdef SHIFT_TO_CENTER
			center = centerOfMass(backproject);
			setCenter(object, center );
			setCenter(max_object, center);
			setCenter(searchWindow, center);
#endif
		}
		trackedPositions.push_back(getBoundingBox(backproject));

		Rect bb_backproject = getBoundingBox(backproject);
		vot_io.outputBoundingBox(bb_backproject);

		if(cnt > 0)
		{
			updateRegion = intersect(max_object + Size(40,40) - Point(20,20), imgRect);
			clocks_start = clock();
			update(ferns, ft, updateRegion, max_object, center, backproject);
			up_clocks += (clock() - clocks_start);
		}
		
	}
	
	std::cout << std::endl << "-- AVERAGE TIMINGS ------------------------------------------" << std::endl;
	std::cout << "feature calculation: " << (((double)ft_clocks) / CLOCKS_PER_SEC / frameNr) << std::endl;
	std::cout << "detection: " << (((double)det_clocks) / CLOCKS_PER_SEC / frameNr) << std::endl;
	std::cout << "backprojection: " << (((double)bp_clocks) / CLOCKS_PER_SEC / frameNr) << std::endl;
	std::cout << "segmentation: " << (((double)seg_clocks) / CLOCKS_PER_SEC / frameNr) << std::endl;
	std::cout << "update: " << (((double)up_clocks) / CLOCKS_PER_SEC / frameNr) << std::endl << std::endl;
}

int main ( int argc, char** argv )
{

	//Check for challenge mode
	for (int i=1; i < argc; i++) {
		if (strcmp(argv[i], "--challenge") == 0) {
			//Enter challenge mode
			run_challenge();
			//End process
			return 0;
		}

	}


	if ( argc < 2 )
	{
		std::cout << "Usage: ./demo <configfilename>" << std::endl;
		exit ( 0 );
	}

	std::string configfilename = std::string ( argv[1] );

	std::cout << std::endl << "-- HOUGH TRACKER ------------------------------------------" << std::endl;
	std::cout << " PARAMETERS: reading " << configfilename << std::endl;

	Parameters hp ( configfilename );
	std::string seq_name = hp.readStringParameter("Tracker.Name");
	float maxScale = static_cast<float>(hp.readDoubleParameter("Tracker.maxScale"));
	float backProjectRadius = 0.5;
	float backProjectminProb = 0.5;

	Capture capture(hp, "Input");
	capture.loadFrame();
	Rect object;
	
	if(hp.settingExists("Tracker.startRegion"))
		object = hp.readRectParameter("Tracker.startRegion");
	else
		object = capture.markRegion();
	
	Mat frame = capture.getFrame();
	namedWindow("Result");

	Rect max_object;
	Size imgSize = Size(frame.cols, frame.rows);
	int baseSize = 12;
	Rect imgRect = Rect(baseSize/2, baseSize/2, imgSize.width-baseSize, imgSize.height-baseSize);
	std::vector<Rect> trackedPositions;
	
	Features ft;
	ft.setImage(frame);
	Mat result(imgSize.height, imgSize.width, CV_32FC1, Scalar(0.0f));

	Mat backproject( imgSize.height, imgSize.width, CV_8UC1, Scalar(GC_BGD) );
	rectangle(backproject, cvPoint(object.x-10, object.y-10), cvPoint(object.x+object.width+10, object.y+object.height+10), Scalar(GC_PR_BGD), -1);
	rectangle(backproject, cvPoint(object.x, object.y), cvPoint(object.x+object.width, object.y+object.height), Scalar(GC_FGD), -1);
	
	Ferns ferns(20, Size(baseSize, baseSize), 8, ft.getNumChannels());
	
	max_object = intersect( imgRect, squarify(object, maxScale));
	double minVal, maxVal = 6.0f;
	Point minLoc, maxLoc;
	Point translation;
	Point center = getCenter(object);
	maxLoc = center;

	std::cout << " INITIAL POSITION: " << object.x << "/" << object.y << " " << object.width << "x" << object.height << std::endl;

	Rect updateRegion = intersect(max_object + Size(40,40) - Point(20,20), imgRect);

	std::cout << " UPDATE Ferns initially: ";
	update(ferns, ft, updateRegion, object, center, backproject);
	trackedPositions.push_back(object);

	mkdir (seq_name.c_str(), 0755);
	Mat display(frame);
	rectangle(display, Point(object.x, object.y), Point(object.x+object.width, object.y+object.height), CV_RGB(0,255,0), 1 );
	std::string filename = createFilename((seq_name + "/init-"), 0, ".jpg");
	imwrite(filename.c_str(), display);

	int frameNr = 0;
	Rect searchWindow = max_object + Size(SEARCH_WINDOW,SEARCH_WINDOW) - Point(SEARCH_WINDOW/2,SEARCH_WINDOW/2);
	
	clock_t ft_clocks = 0;
	clock_t det_clocks = 0;
	clock_t bp_clocks = 0;
	clock_t seg_clocks = 0;
	clock_t up_clocks = 0;
	
	clock_t clocks_start;

	std::cout << " START TRACKING" << std::endl;
	
	while(capture.hasMoreFrames())
	{
		std::cout << std::endl << "-- LOAD FRAME [" << frameNr << "] ------------------------------------------" << std::endl;

		if(capture.loadFrame() == false)
			break;
		
		frame = capture.getFrame();
		
		clocks_start = clock();
		ft.setImage(frame);
		ft_clocks += (clock() - clocks_start);
		result = Scalar(0.0);

		std::cout << " EVALUATE" << std::endl;

		clocks_start = clock();
		ferns.evaluate(ft, intersect(searchWindow, imgRect), result, STEP_WIDTH, 0.5f);
		det_clocks += (clock() - clocks_start);
		
		imshow("Result", result);
		Mat out = result;
		normalize(out, out, 255, 0, NORM_MINMAX);
		
		minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);
		std::cout << " LOCATE: maximum is at (" << maxLoc.x << "/" << maxLoc.y << ": " << maxVal << " )" << std::endl;

		if(maxVal < 3.0f)
		{
			trackedPositions.push_back(object);
			continue;
		}

		translation = Point(maxLoc.x - center.x, maxLoc.y - center.y);
		center = Point(maxLoc.x, maxLoc.y);

		setCenter(max_object, center);
		setCenter(object, center);
		
		setCenter(searchWindow, center);
		
		std::cout << " BACKPROJECT" << std::endl;

		backproject = Scalar(GC_BGD);
		rectangle(backproject, cvPoint(max_object.x, max_object.y), cvPoint(max_object.x+max_object.width, max_object.y+max_object.height), Scalar(GC_PR_BGD), -1);

		clocks_start = clock();
		int cnt = ferns.backProject(ft, backproject, intersect( max_object, imgRect), maxLoc, backProjectRadius, STEP_WIDTH, backProjectminProb);
		bp_clocks += (clock() - clocks_start);
		showSegmentation(backproject, "backProject");

		if(cnt > 0)
		{
			std::cout << " SEGMENT" << std::endl;
			Mat subframe(frame, intersect( searchWindow, imgRect));
			Mat subbackproject(backproject, intersect( searchWindow, imgRect));
			clocks_start = clock();
			Mat fgmdl, bgmdl;
			grabCut(subframe, subbackproject, object, fgmdl, bgmdl, GRABCUT_ROUNDS, GC_INIT_WITH_MASK);
			seg_clocks += (clock() - clocks_start);
		
			showSegmentation(backproject, "Segmentation");
		
			std::cout << " UPDATE: ";

#ifdef SHIFT_TO_CENTER
			center = centerOfMass(backproject);
			setCenter(object, center );
			setCenter(max_object, center);
			setCenter(searchWindow, center);
#endif
		}
		trackedPositions.push_back(getBoundingBox(backproject));
		showResult(frame, backproject, max_object, getBoundingBox(backproject), frameNr++, seq_name);

		if(cnt > 0)
		{
			updateRegion = intersect(max_object + Size(40,40) - Point(20,20), imgRect);
			clocks_start = clock();
			update(ferns, ft, updateRegion, max_object, center, backproject);
			up_clocks += (clock() - clocks_start);
		}
		
		if( cvWaitKey(5) > 0 )
			break;				
	}
	
	std::cout << std::endl << "-- AVERAGE TIMINGS ------------------------------------------" << std::endl;
	std::cout << "feature calculation: " << (((double)ft_clocks) / CLOCKS_PER_SEC / frameNr) << std::endl;
	std::cout << "detection: " << (((double)det_clocks) / CLOCKS_PER_SEC / frameNr) << std::endl;
	std::cout << "backprojection: " << (((double)bp_clocks) / CLOCKS_PER_SEC / frameNr) << std::endl;
	std::cout << "segmentation: " << (((double)seg_clocks) / CLOCKS_PER_SEC / frameNr) << std::endl;
	std::cout << "update: " << (((double)up_clocks) / CLOCKS_PER_SEC / frameNr) << std::endl << std::endl;

	writeResult(trackedPositions, seq_name);
}

void showSegmentation(Mat& backproject, std::string title)
{
	IplImage* display = cvCreateImage(Size(backproject.cols, backproject.rows), IPL_DEPTH_8U, 3 );
	cvZero(display);
	
	for(int x = 0; x < backproject.cols; x++)
		for(int y = 0; y < backproject.rows; y++)
		{
			switch( backproject.at<unsigned char>(y,x) )
			{
				case cv::GC_BGD:
					CV_IMAGE_ELEM( (display), unsigned char, y, x*3+0 ) = 255;
					break;
				case cv::GC_FGD:
					CV_IMAGE_ELEM( (display), unsigned char, y, x*3+2 ) = 255;
					break;
				case cv::GC_PR_BGD:
					CV_IMAGE_ELEM( (display), unsigned char, y, x*3+0 ) = 255;
					CV_IMAGE_ELEM( (display), unsigned char, y, x*3+1 ) = 128;
					CV_IMAGE_ELEM( (display), unsigned char, y, x*3+2 ) = 128;
					break;
				case cv::GC_PR_FGD:
					CV_IMAGE_ELEM( (display), unsigned char, y, x*3+0 ) = 128;
					CV_IMAGE_ELEM( (display), unsigned char, y, x*3+1 ) = 128;
					CV_IMAGE_ELEM( (display), unsigned char, y, x*3+2 ) = 255;
					break;
				default:
					CV_IMAGE_ELEM( (display), unsigned char, y, x*3+1 ) = 128;
			}
		}

	cvNamedWindow(title.c_str());
	cvShowImage(title.c_str(), display);
	cvReleaseImage(&display);
} 


void update(Ferns& ferns, Features& ft, const Rect& ROI, const Rect& object, const Point& center, Mat& mask)
{
	IplImage* updates = cvCreateImage(ft.getSize(), IPL_DEPTH_8U, 3);
	cvZero(updates);

	int numPos = 0;
	int numNeg = 0;
	
	for(int x = ROI.x; x < ROI.x+ROI.width; x+=STEP_WIDTH)
		for(int y = ROI.y; y < ROI.y+ROI.height; y+=STEP_WIDTH)
		{
			if( (mask.at<unsigned char>( y, x ) == GC_FGD) || (mask.at<unsigned char>( y, x ) == GC_PR_FGD) )
			{
				CV_IMAGE_ELEM( updates, unsigned char, y, x*3+2 ) = 255;
				ferns.update(ft, Point(x, y), 1, center);
				numPos++;
			}
			else if(mask.at<unsigned char>( y, x ) == GC_BGD)
			{
				CV_IMAGE_ELEM( updates, unsigned char, y, x*3+0 ) = 255;
				ferns.update(ft, Point(x, y), 0, center);
				numNeg++;
			}
		}
	ferns.forget(0.90);
	cvNamedWindow("Updates");
	cvShowImage("Updates", updates);
	cvReleaseImage(&updates);
	std::cout << "Updated " << (numPos+numNeg) << " points (" << numPos << "+, " << numNeg << "-)" << std::endl;
}

void showResult(Mat& frame, Mat& backproject, const Rect& ROI, const Rect& object, int frameNr, std::string seq_name)
{
	Mat display = frame.clone();

	Mat overlay = backproject.clone();
	
	for(int x = 0; x < overlay.cols; x++)
		for(int y = 0; y < overlay.rows; y++)
		{
			if( (overlay.at<unsigned char>( y, x ) == GC_FGD) || (overlay.at<unsigned char>( y, x ) == GC_PR_FGD) )
				overlay.at<unsigned char>( y, x ) = 1;
			else
				overlay.at<unsigned char>( y, x ) = 0;
		}

	Mat contour = overlay.clone();
	erode(contour, contour, Mat(), Point(-1, -1), 1);
	dilate(contour, contour, Mat(), Point(-1, -1), 5);
	erode(contour, contour, Mat(), Point(-1, -1), 2);
	contour -= overlay;

	for(int x = ROI.x-10; x < ROI.x+ROI.width+10; x++)
		for(int y = ROI.y-10; y < ROI.y+ROI.height+10; y++)
		{
			if(overlay.at<unsigned char>( y, x ) > 0)
			{
				float val = (float)display.at<unsigned char>(y,x*3+2);
				display.at<unsigned char>(y,x*3+2) = static_cast<unsigned char>(val * 0.7f + 255.0f * 0.3f);
			}
			else if(contour.at<unsigned char>( y, x ) > 0)
			{
				display.at<unsigned char>(y,x*3+2) = 255;
				display.at<unsigned char>(y,x*3+1) = 0;
				display.at<unsigned char>(y,x*3+0) = 0;
			}
		}

	namedWindow("Tracking");
	imshow("Tracking", display);
	std::string filename = createFilename((seq_name + "/track-"), frameNr, ".jpg");
	imwrite(filename.c_str(), display);
}

Point centerOfMass(Mat& mask)
{
	float c_x = 0.0f;
	float c_y = 0.0f;
	float c_n = 0.0f;
	
	for(int x = 0; x < mask.cols; x++)
		for(int y = 0; y < mask.rows; y++)
			if( (mask.at<unsigned char>( y, x ) == GC_FGD) || (mask.at<unsigned char>( y, x ) == GC_PR_FGD) )
			{
				c_x += x;
				c_y += y;
				c_n += 1.0f;
			}

	return Point( static_cast<int>(round(c_x/c_n)), static_cast<int>(round(c_y/c_n)));
}

inline Rect squarify(Rect object, double searchFactor)
{
	int len = max(object.width * searchFactor, object.height * searchFactor);
	return Rect( object.x + object.width/2 - len/2, object.y + object.height/2 - len/2, len, len );
}

Rect getBoundingBox(Mat& backproject)
{
	Point min(backproject.cols,backproject.rows);
	Point max(0,0);
	
	for(int x = 0; x < backproject.cols; x++)
		for(int y = 0; y < backproject.rows; y++)
		{
			if( (backproject.at<unsigned char>( y, x ) == GC_FGD) || (backproject.at<unsigned char>( y, x ) == GC_PR_FGD) )
			{
				if(x < min.x)	min.x = x;
				if(y < min.y)	min.y = y;
				if(x > max.x)	max.x = x;
				if(y > max.y)	max.y = y;
			}
		}

	return Rect(min.x, min.y, max.x - min.x, max.y - min.y);
}

void makeBinarySegmentation(Mat& backproject)
{
	for(int x = 0; x < backproject.cols; x++)
		for(int y = 0; y < backproject.rows; y++)
		{
			switch( backproject.at<unsigned char>(y,x) )
			{
				case cv::GC_BGD:
					backproject.at<unsigned char>(y,x) = 0;
					break;
				case cv::GC_FGD:
					backproject.at<unsigned char>(y,x) = 1;
					break;
				case cv::GC_PR_BGD:
					backproject.at<unsigned char>(y,x) = 0;
					break;
				case cv::GC_PR_FGD:
					backproject.at<unsigned char>(y,x) = 1;
					break;
				default:
					backproject.at<unsigned char>(y,x) = 0;
			}
		}
}

void writeResult(std::vector<Rect>& trackedPositions, std::string seq_name)
{
	time_t rawTime = time ( NULL );
	struct tm* locTime = localtime ( &rawTime );
	char timeStamp[80];
	strftime ( timeStamp, 80, "%m%d-%H%M%S", locTime );

	stringstream fileName ( ios_base::out );
	fileName << seq_name << "/result-" << timeStamp << ".txt";

	ofstream  outFile ( fileName.str().c_str(), ios::binary );

	std::cout << "-- WRITING RESULTS -------------------------------------------" << std::endl;
	if ( !outFile )
	{
		cerr << "Could not open \"" << fileName.str() << "\"" << endl;
		exit ( EXIT_FAILURE );
	}

	cout << " Writing file \"" << fileName.str() << "\" ... ";

	std::vector< Rect >::const_iterator it ( trackedPositions.begin() ), end ( trackedPositions.end() );
	while ( it != end )
	{
		outFile << (*it).x << "," << (*it).y << "," << (*it).width << "," << (*it).height << endl;
		++it;
	}

	std::cout << " DONE " << std::endl << std::endl;

	outFile.close();
}
