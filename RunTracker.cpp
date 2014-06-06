/************************************************************************
* File:	RunTracker.cpp
* Brief: C++ demo for paper: Kaihua Zhang, Lei Zhang, Ming-Hsuan Yang,"Real-Time Compressive Tracking," ECCV 2012.
* Version: 1.0
* Author: Yang Xian
* Email: yang_xian521@163.com
* Date:	2012/08/03
* History:
* Revised by Kaihua Zhang on 14/8/2012, 23/8/2012
* Email: zhkhua@gmail.com
* Homepage: http://www4.comp.polyu.edu.hk/~cskhzhang/
* Project Website: http://www4.comp.polyu.edu.hk/~cslzhang/CT/CT.htm
************************************************************************/
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include "CompressiveTracker.h"
#include "vot.hpp"

using namespace cv;
using namespace std;


void readConfig(char* configFileName, char* imgFilePath, Rect &box);
/*  Description: read the tracking information from file "config.txt"
    Arguments:	
	-configFileName: config file name
	-ImgFilePath:    Path of the storing image sequences
	-box:            [x y width height] intial tracking position
	History: Created by Kaihua Zhang on 15/8/2012
*/
void readImageSequenceFiles(char* ImgFilePath,vector <string> &imgNames);
/*  Description: search the image names in the image sequences 
    Arguments:
	-ImgFilePath: path of the image sequence
	-imgNames:  vector that stores image name
	History: Created by Kaihua Zhang on 15/8/2012
*/

int main(int argc, char * argv[])
{

	char imgFilePath[100];
    char  conf[100];
	strcpy(conf,"./config.txt");

	char tmpDirPath[MAX_PATH+1];
	
	// CT framework
	CompressiveTracker ct;
	
	Mat frame;
	Mat grayImg;

	Rect box; // [x y width height] tracking position
	
	//Check if --challenge was passed as an argument
	bool challengeMode = false;
	for (int i = 1; i < argc; i++) {
		if (strcmp("--challenge", argv[i]) == 0) {
			challengeMode = true;
		}
	}

	if (challengeMode) {
		//load region, images and prepare for output
		VOT vot("region.txt", "images.txt", "output.txt");

		Rect box = vot.getInitRectangle();
		
		//output init also bbox
		vot.outputBoundingBox(box);

		vot.getNextImage(frame);
		cvtColor(frame, grayImg, CV_RGB2GRAY);

		ct.init(grayImg, box);    

		while (vot.getNextImage(frame) == 1){
			cvtColor(frame, grayImg, CV_RGB2GRAY);
			ct.processFrame(grayImg, box);// Process frame
			vot.outputBoundingBox(box);
		}
		return 0;
	}

	vector <string> imgNames;
    
	readConfig(conf,imgFilePath,box);
	readImageSequenceFiles(imgFilePath,imgNames);

	sprintf(tmpDirPath, "%s/", imgFilePath);
	imgNames[0].insert(0,tmpDirPath);
	frame = imread(imgNames[0]);
    cvtColor(frame, grayImg, CV_RGB2GRAY);    
	ct.init(grayImg, box);    

	char strFrame[20];

    FILE* resultStream;
	resultStream = fopen("TrackingResults.txt", "w");
	fprintf (resultStream,"%i %i %i %i\n",(int)box.x,(int)box.y,(int)box.width,(int)box.height);

	for(int i = 1; i < imgNames.size()-1; i ++)
	{
		
		sprintf(tmpDirPath, "%s/", imgFilePath);
        imgNames[i].insert(0,tmpDirPath);
        		
		frame = imread(imgNames[i]);// get frame
		cvtColor(frame, grayImg, CV_RGB2GRAY);
		
		ct.processFrame(grayImg, box);// Process frame
		
		rectangle(frame, box, Scalar(200,0,0),2);// Draw rectangle

		fprintf (resultStream,"%i %i %i %i\n",(int)box.x,(int)box.y,(int)box.width,(int)box.height);

		sprintf(strFrame, "#%d ",i) ;

		putText(frame,strFrame,cvPoint(0,20),2,1,CV_RGB(25,200,25));
		
		imshow("CT", frame);// Display
		waitKey(1);		
	}
	fclose(resultStream);

	return 0;
}

void readConfig(char* configFileName, char* imgFilePath, Rect &box)	
{
	int x;
	int y;
	int w;
	int h;

	fstream f;
	char cstring[1000];
	int readS=0;

	f.open(configFileName, fstream::in);

	char param1[200]; strcpy(param1,"");
	char param2[200]; strcpy(param2,"");
	char param3[200]; strcpy(param3,"");

	f.getline(cstring, sizeof(cstring));
	readS=sscanf (cstring, "%s %s %s", param1,param2, param3);

	strcpy(imgFilePath,param3);

	f.getline(cstring, sizeof(cstring)); 
	f.getline(cstring, sizeof(cstring)); 
	f.getline(cstring, sizeof(cstring));


	readS=sscanf (cstring, "%s %s %i %i %i %i", param1,param2, &x, &y, &w, &h);

	box = Rect(x, y, w, h);
	
}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
void readImageSequenceFiles(char* imgFilePath,vector <string> &imgNames)
{	
	imgNames.clear();

	char tmpDirSpec[MAX_PATH+1];
	sprintf (tmpDirSpec, "%s/*", imgFilePath);

	WIN32_FIND_DATA f;
	HANDLE h = FindFirstFile(tmpDirSpec , &f);
	if(h != INVALID_HANDLE_VALUE)
	{
		FindNextFile(h, &f);	//read ..
		FindNextFile(h, &f);	//read .
		do
		{
			imgNames.push_back(f.cFileName);
		} while(FindNextFile(h, &f));

	}
	FindClose(h);	
}
#else
void readImageSequenceFiles(char* imgFilePath,vector <string> &imgNames)
{	
	printf("Not implemented on non-Windows systems\n");
}
#endif
