/*
FragTrack - Fragments-based Tracking Code
-----------------------------------------

By: 	Amit Adam

	amita@cs.technion.ac.il
	www.cs.technion.ac.il/~amita

Date:	November 18'th, 2007

-----------------------------------------
*/

#pragma once

#include <cxcore.h>
#include <cv.h>
#include <highgui.h>

// C++ stuff

#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;

// additional files

#include "emd.h"
#include "vot.hpp"

//
// structs in use
//

//
// Patch - a rectangular patch to be tracked
//

struct Patch
{
	int dx;   // of patch center with respect to template center
	int dy;   // of patch center with respect to template center
	int w;    //patch width is 2*w+1
	int h;    //patch height is 2*h+1
};

//
// Parameters for initializing the tracker
//

struct Parameters
{
	//
	// initial target template
	//

	int initial_tl_y;
	int initial_tl_x;
	int initial_br_y;
	int initial_br_x;

	// search area around position in previous frame

	int search_margin;

	// number of bins in histogram

	int B;

	// metric used for histogram comparison.
	// 1 means chi-square, 2 means EMD, 3 means Kolmogorov-Smirnov variation
	// use 3 (equivalent to 2 and as fast as 1)
	//

	int metric_used;

};

//
// for EMD we need to have the following global function:
//

float my_dist(feature_t* F1, feature_t* F2);

//
// The tracker object:
//

class Fragments_Tracker
{
private:

	//
	// ****************************************************************
	// ****************************************************************
	//
	//    VARIABLES
	//
	// ****************************************************************
	// ****************************************************************
	//
	
	//
	// log file, debugging:
	//

	ofstream& outf;
	int dbg;

	//
	// actual variables
	//

	int handled_frame_number;
	Parameters* params;

	vector < CvMat* > IIV_I;
	vector < CvMat* > IIV_T;
	
	vector < Patch* > patches;
	vector < vector<double>* > template_patches_histograms;
	vector < CvMat* > patch_vote_maps;
	
	CvMat* curr_template;
	int curr_pos_y;
	int curr_pos_x;
	int curr_template_tl_y;
	int curr_template_tl_x;
	int curr_template_height;
	int curr_template_width;

	feature_t* F1;
	feature_t* F2;
	float* w1;
	float* w2;

	//
	// ****************************************************************
	// ****************************************************************
	//
	//    FUNCTIONS
	//
	// ****************************************************************
	// ****************************************************************
	//

public:

	Fragments_Tracker(CvMat* I, Parameters& in_params, ofstream& log_file);

	void Handle_Frame(CvMat* I, char* outwin);
    void Handle_Frame_challenge(CvMat* I, char* outwin, VOT * vot_io);

	~Fragments_Tracker(void);

private:

	void define_patches(int height, int width, vector < Patch* >& patch_vec);
	void Build_Template_Patch_Histograms(CvMat* T,
		                                 vector< vector<double>* >& patch_histograms);


	void Get_Pixel_Bin(CvMat* I, CvMat* bin_mat);
	bool compute_IH(CvMat* I, vector< CvMat* >& vec_II);
	bool compute_histogram(int tl_y, int tl_x, int br_y, int br_x,
		                   vector< CvMat* >& iiv, vector< double >& hist);


	void Init_EMD_Stuff();
	void init_bin_distance_matrix();


	double compare_histograms(vector < double >& hist1, vector < double >& hist2);
	double compare_histograms_emd(vector<double>& h1, vector<double>& h2);
	double compare_histograms_ks( vector < double >& h1 , vector < double >& h2 );


	void compute_single_patch_votes ( Patch* p , vector < double >& hist ,
										   int minrow, int mincol,
										   int maxrow, int maxcol,
										   CvMat* votes, int& min_r, int& min_c,
										   int& max_r, int& max_c);
	void compute_all_patch_votes(vector< vector<double>* >& patch_histograms,
								   vector< Patch* >& tested_patches,
								   int img_height, int img_width,
   								   int minrow, int mincol,
								   int maxrow, int maxcol , CvMat* combined_vote,
								   vector<int>& x_coords,
								   vector<int>& y_coords,
								   vector<double>& patch_scores) ;
	void Combine_Vote_Maps_Median(vector< CvMat* >& vote_maps, CvMat* V);



	void find_template(vector< vector<double>* >& template_histograms,
					   vector< Patch* >& image_patches,
					   int img_height, int img_width,
					   int minrow, int mincol,
					   int maxrow, int maxcol,
					   int& result_y, int& result_x, double& score,
					   vector<int>& x_coords, vector<int>& y_coords);
	void Update_Template(int new_height,int new_width,
						   int new_cy, int new_cx, double scale_factor,
						   CvMat* I);



	void Draw_Rectangle(int tl_y, int tl_x, int height, int width, CvMat* I);

};






