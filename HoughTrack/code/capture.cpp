/******************************************************************************
 * Author: Martin Godec
 *         godec@icg.tugraz.at
 ******************************************************************************/

#include "capture.h"
#include <boost/foreach.hpp>

using namespace std;

void cv_mouse_event ( int event, int x, int y, int flags, void* param )
{
	Capture* c = static_cast<Capture*> ( param );
	{
		switch ( event )
		{
			case CV_EVENT_LBUTTONDOWN:
				c->m_markedRegion.x = x;
				c->m_markedRegion.y = y;
				break;

			case CV_EVENT_LBUTTONUP:
				c->m_markedRegion.width = x - c->m_markedRegion.x;
				c->m_markedRegion.height = y - c->m_markedRegion.y;
				break;

			case CV_EVENT_RBUTTONUP:
				c->m_marked = true;
				break;

			case CV_EVENT_MOUSEMOVE:
				if( flags & CV_EVENT_FLAG_LBUTTON )
				{
					c->m_markedRegion.width = x - c->m_markedRegion.x;
					c->m_markedRegion.height = y - c->m_markedRegion.y;
				}
				break;

			default:
				break;
		}
	}
}

Capture::Capture ( const Parameters &hp, std::string captureName )
: m_increment(1),
  m_scaling(1.0),
  m_loopImages(false),
  m_frameCount(0),
  m_flipVertical(false)
{

	if(hp.settingExists( captureName + ".increment" ))
		m_increment = hp.readIntParameter ( captureName + ".increment" );

	if(hp.settingExists( captureName + ".scaling" ))
		m_scaling = static_cast<float>(hp.readDoubleParameter ( captureName + ".scaling" ));
	
	if(hp.settingExists( captureName + ".loopImages" ) && (hp.readIntParameter ( captureName + ".loopImages" ) == 1) )
		m_loopImages = true;

	if(hp.settingExists(captureName + ".offset"))
		m_frameCount = hp.readIntParameter ( captureName + ".offset" );

	if(hp.settingExists( captureName + ".flipVertical" ) && (hp.readIntParameter ( captureName + ".flipVertical" ) == 1) )
		m_flipVertical = true;

	{
		m_imageSequence = vector<string>();

		m_path = hp.readStringParameter ( captureName + ".path" ) ;

	   string::reverse_iterator It = m_path.rbegin();
		#ifdef WIN32
			if((*It) != '\\')
				m_path.append("\\");
		#else
			if((*It) != '/')
				m_path.append("/");
		#endif


		if ( !boost::filesystem::exists ( m_path ) )
		{
			cout << "Couldn't open " << m_path << endl;
			exit ( EXIT_FAILURE );
		}
		else if ( !boost::filesystem::is_directory ( m_path ) )
		{
			cout << "Couldn't open directory " << m_path << endl;
			exit ( EXIT_FAILURE );
		}
		else
		{
			boost::filesystem::directory_iterator dirItr ( m_path );
			boost::filesystem::directory_iterator endItr;
			std::string imgformat = hp.readStringParameter ( captureName + ".imageFormat" );
			for ( ; dirItr != endItr; ++dirItr )
			{
				std::string fileName = dirItr->path().filename().string();
				if ( fileName.find ( imgformat ) != fileName.npos )
				{
					m_imageSequence.push_back ( fileName );
				}
			}

			assert(m_imageSequence.size() > 0);
			sort ( m_imageSequence.begin(), m_imageSequence.end() );
		}
	}
}

Capture::~Capture()
{
	m_imageSequence.clear();
}

bool Capture::loadFrame()
{
	m_frame = cv::Scalar(0);

	{
		if( m_loopImages == true && m_frameCount >= static_cast<int>(m_imageSequence.size()))
		{
			m_frameCount = 0;
		}

		if (  m_frameCount <= static_cast<int>(m_imageSequence.size()) - 1 )
		{
			m_currentImageName = std::string ( m_path );
			m_currentImageName.append ( m_imageSequence[m_frameCount] );
			m_frame = cv::imread( m_currentImageName.c_str() );
			m_frameCount += m_increment;
		}
	}

	if ( m_frame.data == 0 )
		return false;
	
	if ( m_flipVertical )
		cv::flip(m_frame, m_frame, 1 );

	if( m_scaling != 1.0f )
	{
		cv::Mat orig = m_frame.clone();
		cv::resize(orig, m_frame, cv::Size(orig.cols * m_scaling, orig.rows * m_scaling) );
	}

	return true;
}

bool Capture::hasMoreFrames()
{
	return ((int)m_imageSequence.size() > (m_frameCount+m_increment));
}

cv::Rect Capture::markRegion ( )
{
	cvNamedWindow ( "Mark Object" );

	m_marked = false;
	cvSetMouseCallback ( "Mark Object", cv_mouse_event, this );
	m_markedRegion = cv::Rect(0,0,0,0);

	while ( !m_marked )
	{
		IplImage img = m_frame;
		IplImage* display = cvCloneImage(&img);
		cvRectangle ( display,
		             cvPoint ( m_markedRegion.x, m_markedRegion.y ),
		             cvPoint ( m_markedRegion.x + m_markedRegion.width, m_markedRegion.y + m_markedRegion.height ),
		             cvScalar ( 255.0, 255.0, 255.0 )
		           );
		cvShowImage ( "Mark Object", display );
		int key = cvWaitKey(25);

		if(key == 1048608) // ' '
			loadFrame();

		cvReleaseImage(&display);
	}

	cvDestroyWindow ( "Mark Object" );

	return cv::Rect ( m_markedRegion );
}

