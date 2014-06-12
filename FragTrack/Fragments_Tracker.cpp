/*
FragTrack - Fragments-based Tracking Code
-----------------------------------------

By: 	Amit Adam

	amita@cs.technion.ac.il
	www.cs.technion.ac.il/~amita

Date:	November 18'th, 2007

-----------------------------------------
*/

#include "Fragments_Tracker.h"

//
// global variable required for the EMD calculation
//

CvMat* bin_bin_distance_matrix;

//
// utilities 
//

void Fragments_Tracker::Draw_Rectangle(int tl_y, int tl_x, int height, int width, CvMat* I)
{
	CvPoint tl;
	tl.y = tl_y;
	tl.x = tl_x;
	
	CvPoint br;
	br.y = tl_y + height - 1;
	br.x = tl_x + width - 1;
	
	cvRectangle(I, tl, br, CV_RGB(255,0,0), 3 );
}

//
// Initializing the tracker object:
//

Fragments_Tracker::Fragments_Tracker(CvMat* I, Parameters& in_params, ofstream& log_file) : outf(log_file)
{	
	//outf << "Initializing tracker ... " << endl << endl;
	
	dbg = 0;
	handled_frame_number = 1;

	//
	// real stuff
	//
	
	params = &in_params;

	// for emd computations

	if (params->metric_used == 2)
	{
		Init_EMD_Stuff();
	}

	//
	// allocate space for the IIV_I and the combined votes maps
	//

	CvMat* curr_ii;
	
	int i;
	for (i=0; i<params->B; i++) {
		
		curr_ii = cvCreateMat(I->height,I->width,CV_32S);
		
		IIV_I.push_back(curr_ii);
	}

	//
	// initialize the template
	//

	int t_height = params->initial_br_y - params->initial_tl_y + 1;
	int t_width = params->initial_br_x - params->initial_tl_x + 1;
	
	//
	// define the template - gray-scale:
	//

	CvMat* tmp_ptr = cvCreateMat(t_height,t_width,CV_8U);

	CvRect rect;
	rect.x = params->initial_tl_x;
	rect.y = params->initial_tl_y;
	rect.width = t_width;
	rect.height = t_height;

	// get the pixels 

	cvGetSubRect(I,tmp_ptr,rect);

	curr_template = cvCloneMat(tmp_ptr);
	cvReleaseMat(&tmp_ptr);

	//
	// template center:
	//

	int t_halfw = (int)floor((double)t_width / 2.0 );
	int t_halfh = (int)floor((double)t_height / 2.0 );

    curr_pos_y = params->initial_tl_y + t_halfh;
	curr_pos_x = params->initial_tl_x + t_halfw;
	curr_template_tl_y = params->initial_tl_y;
	curr_template_tl_x = params->initial_tl_x;
	curr_template_height = t_height;
	curr_template_width = t_width;

	//
	// build the histograms of the patches in the template
	//

	Build_Template_Patch_Histograms(curr_template, template_patches_histograms);

	//
	// save the image and the template on it
	//

	Draw_Rectangle(params->initial_tl_y, params->initial_tl_x,
		           t_height, t_width, I);

	cvSaveImage("initial_target.jpg",I);
	cvSaveImage("initial_template.jpg",curr_template);

	//outf << endl << endl << "Finished init... here are the tracking results: " << endl << endl;

	return;
}

//
// when finishing with the tracker release the memory:
//

Fragments_Tracker::~Fragments_Tracker(void)
{
	vector < CvMat* >::iterator it;
	for ( it = IIV_I.begin() ; it != IIV_I.end() ; it++ ) {
		cvReleaseMat ( &(*it) );
	}
	for ( it = IIV_T.begin() ; it != IIV_T.end() ; it++ ) {
		cvReleaseMat ( &(*it) );
	}
	for ( it = patch_vote_maps.begin() ; it != patch_vote_maps.end() ; it++ ) {
		cvReleaseMat ( &(*it) );
	}
	vector < Patch* >::iterator it2;
	for ( it2 = patches.begin() ; it2 != patches.end() ; it2++ ) {
		delete (*it2);
	}

}

//
// Routine for building the histograms of template patches/fragments 
//

void Fragments_Tracker::Build_Template_Patch_Histograms(CvMat* T,
											   vector< vector<double>* >& patch_histograms)
{
	if (dbg==1)
	{
		outf << endl << "Building template patches histograms ... ";
		outf << endl << "Template size: " << T->height << " " << T->width;
	}

	//
	// clear the current histograms
	//

	vector < vector<double>* >::iterator it;
	for (it = patch_histograms.begin(); it != patch_histograms.end(); it++)
	{
		(*it)->clear();
		delete (*it);
	}
	patch_histograms.clear();

	//
	// define the patches on this template
	//

    define_patches(T->height, T->width, patches);

	//
	// compute the integral histogram on the template
	//
	
	int i;
	for (i=0; i<IIV_T.size(); i++) {
		cvReleaseMat(&(IIV_T[i]));
	}
	IIV_T.clear();

	CvMat* curr_ii;
	for (i=0; i<params->B; i++) {

		curr_ii = cvCreateMat(T->height,T->width,CV_32S);
		
		IIV_T.push_back(curr_ii);
	}
	compute_IH ( T , IIV_T );
	
	//
	// now compute the histograms for every defined patch
	//

	vector < double >* curr_histogram;
	
	int t_cx = (int)floor((double)T->width / 2.0 );
	int t_cy = (int)floor((double)T->height / 2.0 );

	if (dbg==1)
	{
		outf << endl << "Template center (in template coords): " << t_cy << " "  << t_cx << " ";
	}

	vector < Patch* >::iterator it2;
	int ctr = 0;
	for ( it2 = patches.begin() ; it2 != patches.end() ; it2++)
	{
		//
		// compute current patch histogram
		//

		if (dbg == 1)
		{
			outf << endl << "Patch number " << ctr << " : dy dx h w = ";
			outf <<  ((*it2)->dy) << " "  << ((*it2)->dx) << " " << ((*it2)->h) << " " << ((*it2)->w);
		}

		int p_cx = t_cx + (*it2)->dx;
		int p_cy = t_cy + (*it2)->dy;
		
		curr_histogram = new vector<double>;
		compute_histogram ( p_cy-(*it2)->h , p_cx-(*it2)->w , p_cy+(*it2)->h , p_cx+(*it2)->w , IIV_T , *curr_histogram );

		
		patch_histograms.push_back(curr_histogram);

		ctr = ctr + 1;

	}

	return;

}

//
// routine for initializing variables used in the EMD metric between two histograms
//
	
void Fragments_Tracker::Init_EMD_Stuff()
{
	int Z = params->B;   // number of bins in histogram

	//
	// alocate vectors
	//

	F1 = new feature_t[256];
	F2 = new feature_t[256];
	w1 = new float[256];
	w2 = new float[256];

	//
	// we assume the bins are numbered 0 to Z-1. We allow at most 256 bins
	// the weights w1,w2 are initialized for every two histograms
	//

	for (int i=0; i<Z; i++)
	{
		F1[i]= i;
		F2[i]= i;
	}

	//
	// compute the distance between every two bins
    //

	bin_bin_distance_matrix = cvCreateMat(Z,Z,CV_32F);
	cvSetZero(bin_bin_distance_matrix);
		
	init_bin_distance_matrix();

	return;
}


void Fragments_Tracker::init_bin_distance_matrix()
{
	vector< double> bin_center;
	double bin_width = floor ( 256. / (double)(params->B) );
	int bi;
	int L,R;
	for (bi=0; bi<=params->B-1 ; bi++)
	{
		L = bi*bin_width;
		R = L+bin_width-1;
		if (bi == params->B - 1)
		{
			R = 255;
		}
		double ctr = 0.5*(L+R);
		bin_center.push_back(ctr);
     }

	/*
	outf << endl << "Gray scale distance matrix: For each channel bin centers are: " << endl;
	for (bi=0; bi < bin_center.size(); bi++)
	{
		outf << endl << bi << " " << bin_center[bi] ;
	}
    */

	int Z = bin_bin_distance_matrix->height;  // number of bins we have
	for (int v1=0; v1<=Z-2; v1++)
	for (int v2=v1+1; v2<=Z-1; v2++)
	{
		double val_b1 = bin_center[v1];
		double val_b2 = bin_center[v2];
		
		double zz = abs(val_b1-val_b2);

		cvSetReal2D(bin_bin_distance_matrix,v1,v2,zz);
		cvSetReal2D(bin_bin_distance_matrix,v2,v1,zz);
	}

	return;

}


//
// define_patches - accepts the template size and returns a vector of patches
// on this window size. Currently the patches are 20 vertical stripes
// each of height half-template-height, and 20 similar horizontal stripes
//

void Fragments_Tracker::define_patches(int height, int width, vector< Patch* >& patch_vec)
{
	for (int i =0 ; i<patch_vec.size(); i++)
	{
		delete patch_vec[i];
	}

	patch_vec.clear();


	//
	// 10 vertical stripes and 10 horizontal stripes
	// each stripe is divided into 2
	//

	int pw_f = (int)floor( ((double) width)*0.25);
	int ph_f = (int)floor( ((double) height)*0.25);

	int pw_s = (int)floor( ((double) width)*0.05 + 0.5);
	int ph_s = (int)floor( ((double) height)*0.05 + 0.5);

	if ( pw_f < 1 ) 	pw_f = 1;
	if ( ph_f < 1 ) 	ph_f = 1;
	if ( pw_s < 1 ) 	pw_s = 1;
	if ( ph_s < 1 ) 	ph_s = 1;
	
	// template center

	int t_cx = (int)floor( ((double) width) / 2.0 );
	int t_cy = (int)floor( ((double) height) / 2.0 );

	int dx = (int)floor( ((double) width) / 4.0 );
	int dy = (int)floor( ((double) height) / 4.0 );

	if (dx > t_cx - pw_f) dx = t_cx-pw_f;
	if (dx > width-1-pw_f-t_cx) dx = width-1-pw_f-t_cx;
	if (dy > t_cy-ph_f) dy = t_cy-ph_f;
	if (dy > height-1-t_cy-ph_f) dy = height-1-t_cy-ph_f;

	Patch* p;

	// horizontal patches
	
	int i;
	for (i = ph_s; i<=height-1-ph_s; i = i + 2*ph_s)
	{
		p = new Patch;
		p->dy = i-t_cy;
		p->dx = -dx;
		p->h = ph_s;
		p->w = pw_f;
		patch_vec.push_back(p);

		p = new Patch;
		p->dy = i-t_cy;
		p->dx = dx;
		p->h = ph_s;
		p->w = pw_f;
		patch_vec.push_back(p);
	}

	//
	// vertical patches
	//

	for (i = pw_s; i<=width-1-pw_s; i=i+2*pw_s)
	{
		p = new Patch;
		p->dx = i-t_cx;
		p->dy = -dy;
		p->w = pw_s;
		p->h = ph_f;
		patch_vec.push_back(p);

		p = new Patch;
		p->dx = i-t_cx;
		p->dy = dy;
		p->w = pw_s;
		p->h = ph_f;
		patch_vec.push_back(p);
	}

	return;
}

//
// Get_Pixel_Bin - routine which bins the image I and returns the result
// in bin_mat. 
//

void Fragments_Tracker::Get_Pixel_Bin ( CvMat* I , CvMat* bin_mat)
{
	double bin_width = floor ( 256. / (double)(params->B) );
	
	for (int row=0; row<bin_mat->height; row++)
	for (int col=0; col<bin_mat->width; col++)
	{
		int b = (int)(floor ( cvGetReal2D ( I , row , col ) / bin_width ));
		if ( b > (params->B - 1) ) 
			b = params->B - 1;

		cvmSet(bin_mat,row,col,b);
	}

	return;
}

//
// compute_IH - compute integral histogram. Also possible to use OpenCV's 
// routine, however there is a difference in the size of matrices returned
//

bool Fragments_Tracker::compute_IH( CvMat* I , vector < CvMat* >& vec_II )
{
	int save_dbg = dbg;
	//dbg = 0;

	//reset IIV matrices
	vector < CvMat* >::iterator it;
	for ( it = vec_II.begin() ; it != vec_II.end() ; it++ ) {
		cvSetZero ( (*it) );
	}


	CvMat* curr_bin_mat = cvCreateMat(I->height,I->width,CV_32F);
	Get_Pixel_Bin ( I , curr_bin_mat);

	//fill matrices
	int i , j , currBin, count;
	double vup , vleft , vdiag , z;
	for ( i = 0 ; i < I->height ; i++ ) {
		for ( j = 0 ; j < I->width ; j++ ) {


			currBin = cvmGet(curr_bin_mat,i,j);

			for ( it = vec_II.begin() , count = 0 ; it != vec_II.end() ; it++ , count++ ) {
				
				if ( i == 0 ) {//no up
					vup = 0;
					vdiag = 0;
				} else {
					vup = cvGetReal2D ( (*it) , i-1 , j );
					//vup = cvmGet ( (*it) , i-1 , j );
				}
				
				if ( j == 0 ) {//no left
					vleft = 0;
					vdiag = 0;
				} else {
					vleft = cvGetReal2D ( (*it) , i , j-1 );
					//vleft = cvmGet ( (*it) , i , j-1 );
				}

				if ( i > 0 && j > 0 ) {//diag exists
					vdiag = cvGetReal2D ( (*it) , i-1 , j-1 );
					//vdiag = cvmGet ( (*it) , i-1 , j-1 );
				}

				//set cell value
				z = vleft + vup - vdiag;
				if ( currBin == count ) 
					z++;
				cvSetReal2D ( (*it) , i , j , (double)z );
				//cvmSet ( (*it) , i , j , (double)z );

			}//next it
		}//next j
	}//next i

	cvReleaseMat(&curr_bin_mat);

	dbg = save_dbg;
	return true;
}

//
// compute_histogram - uses the integral histogram data structure to quickly compute
// a histogram in a rectangular region
//

bool Fragments_Tracker::compute_histogram ( int tl_y , int tl_x , int br_y , int br_x , vector < CvMat* >& iiv , vector < double >& hist )
{
	vector < CvMat* >::iterator it;
	hist.clear();
	double left , up , diag;
	double sum = 0;
	double z;
	for ( it = iiv.begin() ; it != iiv.end() ; it++ ) {
		if ( tl_x == 0 ) {
			left = 0;
			diag = 0;
		} else {
			left = cvGetReal2D ( (*it) , br_y , tl_x - 1 );
			//left = cvmGet ( (*it) , br_y , tl_x - 1 );
		}

		if ( tl_y == 0 ) {
			up = 0;
			diag = 0;
		} else {
			up = cvGetReal2D ( (*it) , tl_y - 1 , br_x );
			//up = cvmGet ( (*it) , tl_y - 1 , br_x );
		}
		if ( tl_x > 0 && tl_y > 0 ) {
			diag = cvGetReal2D ( (*it) , tl_y - 1 , tl_x - 1 );
			//diag = cvmGet ( (*it) , tl_y - 1 , tl_x - 1 );
		}
		z = cvGetReal2D ( (*it) , br_y , br_x ) - left - up + diag;
		//z = cvmGet ( (*it) , br_y , br_x ) - left - up + diag;
		hist.push_back(z);
		sum += z;
	}
	vector < double >::iterator it2;
	for ( it2 = hist.begin() ; it2 != hist.end() ; it2++ ) {
		(*it2) /= sum;
	}
	return true;
}

//
// my_dist - the routine used by EMD to compute distance between two bins
//

float my_dist(feature_t* F1, feature_t* F2)
{
	return ((float) cvGetReal2D(bin_bin_distance_matrix,(*F1),(*F2)));
}

//
// compare_histograms - returns their chi-square distance
//

double Fragments_Tracker::compare_histograms ( vector < double >& h1 , vector < double >& h2 )
{
	double sum = 0;
	double z,y;
	vector < double >::iterator it1 , it2;
	for ( it1 = h1.begin() , it2 = h2.begin() ; it1 != h1.end() , it2 != h2.end() ; it1++ , it2++ ) {
		z = (*it1) - (*it2);
		y = (*it1) + (*it2);
		if (z != 0)
		{
			sum += z*z/y; 
		}
	}
	return sum;
}

//
// compare_histograms_emd - returns their EMD distance
// Uses Yossi Rubner's original code
//

double Fragments_Tracker::compare_histograms_emd(vector<double>& h1, vector<double>& h2)
{
	//
	// copy the vectors into the arrays
	//

	int Z = h1.size();    // number of bins
	for (int i=0; i<Z; i++)
	{
		w1[i]=(float) h1[i];
		w2[i]=(float) h2[i];
	}

	//
	// define "signature"
	//

	signature_t s1 = { Z, F1, w1};
	signature_t s2 = { Z, F2, w2};

	float E = emd(&s1, &s2, my_dist, 0, 0);

	return E;

}

//
// compare_histograms_ks - 
// Uses a variation of the  Kolmogorov-Smirnov statistic to compare two
// histograms. For one-dimensional data this is exactly equivalent to EMD
// but much faster of course
// 
// Amit January 21'st 2008
//

double Fragments_Tracker::compare_histograms_ks( vector < double >& h1 , vector < double >& h2 )
{
	double sum = 0;
	double cdf1 = 0;
	double cdf2 = 0;
	double z;
	double ctr = 0;
	vector < double >::iterator it1 , it2;
	for ( it1 = h1.begin() , it2 = h2.begin() ; it1 != h1.end() , it2 != h2.end() ; it1++ , it2++ ) {
		cdf1 = cdf1 + (*it1);
		cdf2 = cdf2 + (*it2);

		z = cdf1 - cdf2;
		sum += abs(z);
		ctr = ctr + 1;
	}
	return (sum/ctr);
}

//
// compute_single_patch_votes - computes the votes map associated with a single patch
//

void Fragments_Tracker::compute_single_patch_votes ( Patch* p , vector < double >& hist ,
										   int minrow, int mincol,
										   int maxrow, int maxcol,
										   CvMat* votes, int& min_r, int& min_c,
										   int& max_r, int& max_c)
{
	int save_dbg = dbg;
	dbg = 0;

	if (dbg==1)
	{
		outf << endl << "Entering single patch votes ... " << endl;
		outf << endl << "Patch center dy dx  = " << p->dy << " " << p->dx << " patch h w = " << p->h << " " << p->w;
		outf << endl << "Voting for target center in region [" << minrow << "," << maxrow << "]x[" << mincol << "," << maxcol << "]" << endl;
	}

	int M = (*IIV_I.begin())->height;
	int N = (*IIV_I.begin())->width;
	int minx , maxx , miny, maxy;
	//compute left margin
	if ( p->w > p->dx ) {
		minx = p->w;
	} else
		minx = p->dx;

	//compute right margin
	if ( p->dx < -p->w ) {
		maxx = N-1+p->dx;
	} else
		maxx = N-1-p->w;

	//compute up margin
	if ( p->h > p->dy ) {
		miny = p->h;
	} else
		miny = p->dy;

	//compute bottom margin
	if ( p->dy < -p->h ) {
		maxy = M-1+p->dy;
	} else
		maxy = M-1-p->h;

	//
	// patch center (y,x) votes for the target center at (y-dy,x-dx)
	// we want votes only in the range min/max-row/col
	// we now enforce it:
	//      

	if (miny<minrow+p->dy) {miny = minrow+p->dy;}
	if (maxy>maxrow+p->dy) {maxy = maxrow+p->dy;}
	if (minx<mincol+p->dx) {minx = mincol+p->dx;}
	if (maxx>maxcol+p->dx) {maxx = maxcol+p->dx;}

	cvSet( votes, cvScalar(1000.0) );
	int x , y;
	double z = 0;
	double sum_z = 0;
	double t = 0;
	vector < double > curr_hist;

	if (dbg == 1)
	{
		outf << endl << "patch center runs  in the region [" << miny << "," << maxy << "]x[" << minx << "," << maxx << "]" << endl;
	}

	for ( x = minx ; x <= maxx ; x++ ) {
		for ( y = miny ; y <= maxy ; y++ ) {

			compute_histogram ( y-p->h , x-p->w , y+p->h , x+p->w, IIV_I , curr_hist );
			

			//
			// compare the two histograms
			//

			if (params->metric_used == 1) z = compare_histograms(hist,curr_hist);
			if (params->metric_used == 2) z = compare_histograms_emd(hist,curr_hist);
			if (params->metric_used == 3) z = compare_histograms_ks(hist,curr_hist);
			
			if (dbg == 1)
			{
				outf << endl << "The distance between this hist and the template hist is " << z << endl;
			}
			
			//
			// now the votemap is not the whole image but only the portion between
			// min-max row-col
			// so y-dy=minrow --> vote for index = 0
			// 

			cvSetReal2D ( votes , y-p->dy-minrow , x-p->dx-mincol , z );

		}
	}

	//
	// return the region where votes were added
	//

	min_c = minx-p->dx;
	max_c = maxx-p->dx;
	min_r = miny-p->dy;
	max_r = maxy-p->dy;

	dbg = save_dbg;
	return;
}

//
// compute_all_patch_votes - runs on all patches and computes each one's vote map
// Then combines all the votes maps (robustly) to a single vote map
//

void Fragments_Tracker::compute_all_patch_votes(vector< vector<double>* >& patch_histograms,
										 vector< Patch* >& tested_patches,
										 int img_height, int img_width,
   										 int minrow, int mincol,
										 int maxrow, int maxcol , CvMat* combined_vote,
										 vector<int>& x_coords,
										 vector<int>& y_coords,
										 vector<double>& patch_scores) 
{
	int save_dbg = dbg;

	if (dbg == 1)
	{
		outf << endl << "Frame num " << handled_frame_number << " - entering compute all patch votes 2: ";
	}

	//
	// pass on every patch and build its vote map
	// note: we allocate the vote map matrices in this loop
	//

	vector < Patch* >::iterator it;
	
	CvMat* curr_vm;
	vector < double >* curr_patch_histogram;

	patch_vote_maps.clear();
	x_coords.clear();
	y_coords.clear();
	patch_scores.clear();

	vector<int> vote_regions_minrow; 
	vector<int> vote_regions_mincol;
	vector<int> vote_regions_maxrow;
	vector<int> vote_regions_maxcol;

	int minx,miny,maxx,maxy;

	int vm_width = maxcol-mincol+1;
	int vm_height = maxrow-minrow+1;

	int i = -1;
	for ( it = tested_patches.begin() ; it != tested_patches.end() ; it++) {

		curr_vm = cvCreateMat(vm_height,vm_width,CV_32F);

		//
		// compute current patch histogram
		//
		
		i = i + 1;
		curr_patch_histogram = patch_histograms[i];

		if (dbg==1)
		{
			outf << endl << "The current patch number i= " << i << " histogram is : " << endl;
			//util_object.PrintVec_to_stream(*curr_patch_histogram,outf);
			outf << endl << endl << "Going to vote with this patch ... ";
		}

		compute_single_patch_votes ( (*it) , *curr_patch_histogram , minrow, mincol,
							          maxrow, maxcol, curr_vm, miny, minx, maxy, maxx );

		patch_vote_maps.push_back(curr_vm);
		vote_regions_minrow.push_back(miny);
		vote_regions_mincol.push_back(minx);
		vote_regions_maxrow.push_back(maxy);
		vote_regions_maxcol.push_back(maxx);

		//
		// find the position based on this patch:
		//

		CvPoint min_loc;
		CvPoint max_loc;
		double minval;
		double maxval;

		cvMinMaxLoc(curr_vm,&minval,&maxval,&min_loc,&max_loc,NULL);
		
		x_coords.push_back(mincol+min_loc.x);
		y_coords.push_back(minrow+min_loc.y);
		patch_scores.push_back(minval);

		if (dbg == 1)
		{
			outf << endl << "Patch " << i << " votes for point yx = " << min_loc.y << " " << min_loc.x << " (in vote-map coords) with score = " << minval;
			outf << endl << "In image coords this is  point yx = " << (minrow+min_loc.y) << " " << (mincol+min_loc.x);
			outf << endl << "Moving on to next patch ... " << endl;
		}

	}  // next patch

	//
	// combine patch votes - using a quantile based score makes this combination
	// robust to occlusions
	//

	Combine_Vote_Maps_Median(patch_vote_maps, combined_vote);

	//
	// release stuff 
	//

	vector< CvMat* >::iterator vm_it;
	for (vm_it = patch_vote_maps.begin(); vm_it != patch_vote_maps.end(); vm_it++)
	{
		cvReleaseMat(&(*vm_it));
	}
	patch_vote_maps.clear();

	dbg = save_dbg;
	return;
}

//
// Combine_Vote_Maps_Median - at each hypothesis sorts the score given by each patch
// and takes the Q'th quantile as the score. This ignores outlier scores contributed
// by patches affected by occlusions for example
//

void Fragments_Tracker::Combine_Vote_Maps_Median(vector< CvMat* >& vote_maps, CvMat* V)
{
	int M = (vote_maps[0])->height;
	int N = (vote_maps[0])->width;

	int Z = vote_maps.size();
	
	vector< double > Fv;

	int Q_index = (int) floor(((double) Z) / 4.0);
	//Q_index = 4;

	if (dbg == 1)
	{
		outf << endl << "Combine votes median - frame = " << handled_frame_number;
		outf << endl << "Vote map size = " << M << " " << N;
		outf << endl << "Number of vote maps: " << Z << " taking quantile " << Q_index;
		outf << flush;
	}

	for (int i=0; i<M; i++)
	for (int j=0; j<N; j++)
	{
		// take all the values this pixel got in all the vote maps
		// and compute their median

		Fv.clear();

		for (int p=0; p < Z; p++)
		{
			double z = cvGetReal2D(vote_maps[p],i,j);
			Fv.push_back(z);
		}

		std::sort(Fv.begin(),Fv.end());
		
		double Q = Fv[Q_index];
		cvSetReal2D(V,i,j,Q);
	}

	if (dbg == 1)
	{
		outf << endl << "The resulting median vote matrix: ";
		//util_object.PrintMat_to_stream(V,outf);
		outf << endl << " ********** " << endl;
		outf << flush;
	}

	return;
}

//
// find_template - the main routine. Searches for the template in the region defined
// by minrow,mincol and maxrow,maxcol.
// Returns the result in result_y,result_x and with it its associated score
//

void Fragments_Tracker::find_template(vector< vector<double>* >& template_histograms,
							 vector< Patch* >& image_patches,
							 int img_height, int img_width,
							 int minrow, int mincol,
							 int maxrow, int maxcol,
							 int& result_y, int& result_x, double& score,
							 vector<int>& x_coords,
							 vector<int>& y_coords)
{
	if (minrow<0) {minrow = 0;}
	if (mincol<0) {mincol = 0;}
	if (maxrow>=img_height) {maxrow = img_height-1;}
	if (maxcol>=img_width) {maxcol = img_width-1;}

	// in handle frame we already computed the integral histogram
	// we have it updated in IIV_I
	
	vector<double> p_scores;

	CvMat* combined_vote = cvCreateMat(maxrow-minrow+1,maxcol-mincol+1,CV_32F);

	compute_all_patch_votes(template_histograms, image_patches , img_height, img_width,
		                      minrow, mincol, maxrow, maxcol,
							  combined_vote,
							  x_coords,
							  y_coords,
							  p_scores);

	CvPoint min_loc;
	CvPoint max_loc;
	double minval;
	double maxval;

	cvMinMaxLoc(combined_vote,&minval,&maxval,&min_loc,&max_loc,NULL);
	int cx = min_loc.x;
	int cy = min_loc.y;

	result_y = cy + minrow;
	result_x = cx + mincol;
	score = minval;

	if (dbg==1)
	{
		outf << endl << "In find-template: we searched in the range [" << minrow << "," << maxrow << "]x[" << mincol << "," << maxcol << "] The combined vote map is: " << endl;
		//util_object.PrintMat_to_stream(combined_vote,outf);
		outf << endl << endl << "The min location is: " << result_y << " " << result_x;
		outf << endl << endl << "The min value in this location is: " << score;
		outf << endl << "Exiting find template ";
	}

	cvReleaseMat(&combined_vote);

	return;
}

//
// Update_Template - updates the template's position and makes sure it stays
// inside the image
//

void Fragments_Tracker::Update_Template(int new_height,int new_width,
								 int new_cy, int new_cx, double scale_factor,
								 CvMat* I)
{
	curr_pos_y = new_cy;
	curr_pos_x = new_cx;

	int t_halfw = (int)floor((double)new_width / 2.0 );
	int t_halfh = (int)floor((double)new_height / 2.0 );

	curr_template_tl_y = curr_pos_y - t_halfh;
	curr_template_tl_x = curr_pos_x - t_halfw;
	
	curr_template_height = new_height;
	curr_template_width = new_width;

	//
	// make sure we stay inside the image (without changing template size)
	//

	if (curr_template_tl_y < 0) {curr_template_tl_y = 0;}
	if (curr_template_tl_x < 0) {curr_template_tl_x = 0;}
	if (curr_template_tl_y > I->height-new_height)
	{
		curr_template_tl_y = I->height-new_height;
	}
	if (curr_template_tl_x > I->width-new_width)
	{
		curr_template_tl_x = I->width-new_width;
	}
	
	return;

}

//
// Handle_Frame - the outside interface after tracker is initialized. Call it with the
// current frame and get the output in the window outwin, and in the log file
//

void Fragments_Tracker::Handle_Frame(CvMat* I, char* outwin)
{

	handled_frame_number = handled_frame_number + 1;
	
	//
	// build this image's IH. From now on, we only work with
	// this data structure and not with the image itself
	//
	
	int img_height;
	int img_width;
	
	compute_IH (I, IIV_I );
	img_height = I->height;
	img_width = I->width;
	
	//
	// find the current template in the current image
	//

	vector<int> x_coords;
	vector<int> y_coords;

	int new_yM, new_xM;
	double score_M;

	find_template(template_patches_histograms,
				  patches,
		          img_height, img_width,
			      curr_pos_y - (params->search_margin),
				  curr_pos_x - (params->search_margin),
				  curr_pos_y + (params->search_margin),
				  curr_pos_x + (params->search_margin),
				  new_yM, new_xM, score_M,
				  x_coords, y_coords);


	Update_Template(curr_template->height,curr_template->width,new_yM,new_xM,1,I);

	//
	// finished. now output the results
	//

	//
	// output tracking position
	//

	if (handled_frame_number == 2)
	{
		outf << endl << "Tracking results - the format is as follows: " << endl << endl << "handled_frame_number   center_y   center_x   score   topleft_y   topleft_x   height   width " << endl << endl;
	}

	outf << endl << handled_frame_number << " " << curr_pos_y << " " << curr_pos_x << " " << score_M << " " << curr_template_tl_y << " " << curr_template_tl_x << " " << curr_template_height << " " << curr_template_width;

	//
	// output the results
	//

	//
	// in black and white:
	//

	CvMat* out_mat = cvCreateMat(I->height,I->width,CV_8UC3);
	cvCvtPlaneToPix(I,I,I,NULL,out_mat);   // get a gray scale but color image

	Draw_Rectangle(curr_template_tl_y, curr_template_tl_x,
		           curr_template_height, curr_template_width, out_mat);

	cvShowImage(outwin,out_mat);
	cvReleaseMat(&out_mat);

	return;

}

void Fragments_Tracker::Handle_Frame_challenge(CvMat* I, char* outwin, VOT * vot_io)
{

	handled_frame_number = handled_frame_number + 1;
	
	//
	// build this image's IH. From now on, we only work with
	// this data structure and not with the image itself
	//
	
	int img_height;
	int img_width;
	
	compute_IH (I, IIV_I );
	img_height = I->height;
	img_width = I->width;
	
	//
	// find the current template in the current image
	//

	vector<int> x_coords;
	vector<int> y_coords;

	int new_yM, new_xM;
	double score_M;

	find_template(template_patches_histograms,
				  patches,
		          img_height, img_width,
			      curr_pos_y - (params->search_margin),
				  curr_pos_x - (params->search_margin),
				  curr_pos_y + (params->search_margin),
				  curr_pos_x + (params->search_margin),
				  new_yM, new_xM, score_M,
				  x_coords, y_coords);


	Update_Template(curr_template->height,curr_template->width,new_yM,new_xM,1,I);

	//
	// finished. now output the results
	//

	//
	// output tracking position
	//

	if (handled_frame_number == 2)
	{
		outf << endl << "Tracking results - the format is as follows: " << endl << endl << "handled_frame_number   center_y   center_x   score   topleft_y   topleft_x   height   width " << endl << endl;
	}

	outf << endl << handled_frame_number << " " << curr_pos_y << " " << curr_pos_x << " " << score_M << " " << curr_template_tl_y << " " << curr_template_tl_x << " " << curr_template_height << " " << curr_template_width;

	cv::Rect output = cv::Rect(curr_template_tl_x, curr_template_tl_y, curr_template_width, curr_template_height);

	vot_io->outputBoundingBox(output);

	return;

}
