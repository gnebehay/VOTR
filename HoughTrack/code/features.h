/******************************************************************************
 * Author: Martin Godec
 *         godec@icg.tugraz.at
 * Feature calculation from Juergen Gall
 *         gall@vision.ee.ethz.ch
 ******************************************************************************/

#ifndef FEATURES_H_
#define FEATURES_H_

#include <cxcore.h>
#include <cv.h>
#include <vector>
#include "utilities.h"

class HoG {
public:
	HoG();
	~HoG()
	{
		cvReleaseMat(&Gauss);
		delete ptGauss;
	}

	void extractOBin(IplImage *Iorient, IplImage *Imagn, std::vector<IplImage*>& out, int off);

private:

	void calcHoGBin(uchar* ptOrient, uchar* ptMagn, int step, double* desc);
	void binning(float v, float w, double* desc, int maxb);

	int bins;
	float binsize; 

	int g_w;
	CvMat* Gauss;

	// Gauss as vector
	float* ptGauss;
};

inline void HoG::calcHoGBin(uchar* ptOrient, uchar* ptMagn, int step, double* desc) {
	for(int i=0; i<bins;i++)
		desc[i]=0;

	uchar* ptO = &ptOrient[0];
	uchar* ptM = &ptMagn[0];
	int i=0;
	for(int y=0;y<g_w; ++y, ptO+=step, ptM+=step) {
		for(int x=0;x<g_w; ++x, ++i) {
			binning((float)ptO[x]/binsize, (float)ptM[x] * ptGauss[i], desc, bins);
		}
	}
}

inline void HoG::binning(float v, float w, double* desc, int maxb) {
	int bin1 = int(v);
	int bin2;
	float delta = v-bin1-0.5f;
	if(delta<0) {
		bin2 = bin1 < 1 ? maxb-1 : bin1-1; 
		delta = -delta;
	} else
		bin2 = bin1 < maxb-1 ? bin1+1 : 0; 
	desc[bin1] += (1-delta)*w;
	desc[bin2] += delta*w;
}

class Features
{
public:
	Features()
	{
		m_original = 0;
        m_numChannels = 16;
    };
    
	~Features()
	{
	    for(unsigned int c = 0; c < m_channels.size(); c++)
	    	cvReleaseImage(&m_channels.at(c));
		
	    m_channels.clear();
    }

	inline void setImage(Mat& img)
	{
		if(m_original == 0)
			m_original = cvCreateImage(cvSize(img.cols, img.rows), IPL_DEPTH_8U, 3);
		
		(*m_original) = img;

	    for(unsigned int c = 0; c < m_channels.size(); c++)
		    cvReleaseImage(&m_channels.at(c));
		
	    m_channels.resize(m_numChannels);

	    extractFeatureChannels(m_original, m_channels);
    };
    
	inline const IplImage* getChannel(unsigned int idx) const
	{
    	return m_channels.at(idx);
    };

	inline unsigned int getNumChannels() const
	{
	    return m_numChannels;
    };
    
    inline Size getSize() const
    {
        if(m_original != 0)
            return Size(m_original->width, m_original->height);
        else
            return Size(0,0);
    };
	
private:
	unsigned int m_numChannels;
	IplImage* m_original;
	std::vector< IplImage* > m_channels;
	HoG hog;

	inline void extractFeatureChannels(const IplImage *img, std::vector<IplImage*>& vImg)
	{
	    // 16 feature channels
	    // 7+9 channels: L, a, b, |I_x|, |I_y|, |I_xx|, |I_yy|, HOGlike features with 9 bins (weighted orientations 5x5 neighborhood)

	    vImg.resize(16);
	    for(unsigned int c=0; c<vImg.size(); ++c)
		    vImg[c] = cvCreateImage(cvSize(img->width,img->height), IPL_DEPTH_8U , 1);

	    cvCvtColor( img, vImg[0], CV_RGB2GRAY );
	    cvSmooth( vImg[0], vImg[0], CV_MEDIAN);

	    // Temporary images for computing I_x, I_y (Avoid overflow for cvSobel)
	    IplImage* I_x = cvCreateImage(cvSize(img->width,img->height), IPL_DEPTH_16S, 1);
	    IplImage* I_y = cvCreateImage(cvSize(img->width,img->height), IPL_DEPTH_16S, 1);

	    // |I_x|, |I_y|
	    cvSobel(vImg[0],I_x,1,0,3);
	    cvSobel(vImg[0],I_y,0,1,3);

	    cvConvertScaleAbs( I_x, vImg[3], 0.25);
	    cvConvertScaleAbs( I_y, vImg[4], 0.25);

	    //#pragma omp sections nowait
	    {
		    //#pragma omp section
		    {

		    short* dataX;
		    short* dataY;
		    uchar* dataZ;
		    int stepX, stepY, stepZ;
		    CvSize size;
		    int x, y;

		    cvGetRawData( I_x, (uchar**)&dataX, &stepX, &size);
		    cvGetRawData( I_y, (uchar**)&dataY, &stepY);
		    cvGetRawData( vImg[1], (uchar**)&dataZ, &stepZ);
		    stepX /= sizeof(dataX[0]);
		    stepY /= sizeof(dataY[0]);
		    stepZ /= sizeof(dataZ[0]);

		    // Orientation of gradients
		    for( y = 0; y < size.height; y++, dataX += stepX, dataY += stepY, dataZ += stepZ  )
			    for( x = 0; x < size.width; x++ ) {
				    // Avoid division by zero
				    float tx = (float)dataX[x] + (0.000001f * sign((float)dataX[x]));
				    // Scaling [-pi/2 pi/2] -> [0 80*pi]
				    dataZ[x]=uchar( ( atan((float)dataY[x]/tx)+3.14159265f/2.0f ) * 80 );
			    }
		    }

		    //#pragma omp section
		    {
			    short* dataX;
			    short* dataY;
			    uchar* dataZ;
			    int stepX, stepY, stepZ;
			    CvSize size;
			    int x, y;

			    cvGetRawData( I_x, (uchar**)&dataX, &stepX, &size);
			    cvGetRawData( I_y, (uchar**)&dataY, &stepY);
			    cvGetRawData( vImg[2], (uchar**)&dataZ, &stepZ);
			    stepX /= sizeof(dataX[0]);
			    stepY /= sizeof(dataY[0]);
			    stepZ /= sizeof(dataZ[0]);

			    // Magnitude of gradients
			    for( y = 0; y < size.height; y++, dataX += stepX, dataY += stepY, dataZ += stepZ  )
				    for( x = 0; x < size.width; x++ ) {
					    dataZ[x] = (uchar)( sqrt((float)dataX[x]*(float)dataX[x] + (float)dataY[x]*(float)dataY[x]) );
				    }
			    }

		    //#pragma omp section
		    {
			    // 9-bin HOG feature stored at vImg[7] - vImg[15]
			    hog.extractOBin(vImg[1], vImg[2], vImg, 7);
		    }

		    //#pragma omp section
		    {
			    // |I_xx|, |I_yy|

			    cvSobel(vImg[0],I_x,2,0,3);
			    cvConvertScaleAbs( I_x, vImg[5], 0.25);

			    cvSobel(vImg[0],I_y,0,2,3);
			    cvConvertScaleAbs( I_y, vImg[6], 0.25);
		    }

		    //#pragma omp section
		    {
			    // L, a, b
			    IplImage* img_lab = cvCreateImage(cvSize(img->width,img->height), IPL_DEPTH_8U, 3);
			    cvCvtColor( img, img_lab, CV_RGB2Lab  );

			    cvSplit( img_lab, vImg[0], vImg[1], vImg[2], 0);

			    cvSmooth( vImg[0], vImg[0], CV_MEDIAN);
			    cvSmooth( vImg[1], vImg[1], CV_MEDIAN);
			    cvSmooth( vImg[2], vImg[2], CV_MEDIAN);

			    cvReleaseImage(&img_lab);
		    }
	    }

	    cvReleaseImage(&I_x);
	    cvReleaseImage(&I_y);
    };


};

#endif //FEATURES_H_
