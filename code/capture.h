/******************************************************************************
 * Author: Martin Godec
 *         godec@icg.tugraz.at
 ******************************************************************************/

#ifndef CAPTURE_H_
#define CAPTURE_H_

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <sys/types.h>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include "cv.h"
#include "highgui.h"
#include "cvaux.h"

#include "parameters.h"

#define DEFAULT_LINE_THICKNESS	2
#define DEFAULT_LINE_COLOR	CV_RGB(255,0,0)

#define MARK_TYPE_FLAG 1
#define MARK_TYPE_RECT 0
#define MARK_TYPE_POINT 1

#define MARK_TYPE_FIX_X 2
#define MARK_TYPE_FIX_Y 4
#define MARK_TYPE_FIX_POSITION 6

#define MARK_TYPE_FIX_WIDTH 8
#define MARK_TYPE_FIX_HEIGHT 16
#define MARK_TYPE_FIX_SIZE 24

using namespace std;

class Capture {
public:
	Capture(const Parameters &hp, string captureName = "Data" );
	virtual ~Capture();

	bool loadFrame();
	cv::Mat getFrame() { return m_frame; }
	bool hasMoreFrames();
	int getNumSamples() { return m_imageSequence.size(); }

	cv::Rect markRegion();

	friend void cv_mouse_event (int event, int x, int y, int flags, void* param);
	inline string getCurrentImageName() const { return m_currentImageName; }

protected:
	cv::Mat m_frame;
	int m_increment;
	float m_scaling;

	std::string m_path;
	vector<string> m_imageSequence;

	bool m_loopImages;
	int m_frameCount;
	string m_currentImageName;
	bool m_flipVertical;

	cv::Rect m_markedRegion;
	bool m_marked;

};

#endif // CAPTURE_H_
