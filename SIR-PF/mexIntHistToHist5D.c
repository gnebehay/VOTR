#include "mex.h"

#define index5D(r, c, b1, b2, b3, R, C, B1, B2, B3) (b3)*(R)*(C)*(B1)*(B2) + (b2)*(R)*(C)*(B1) + (b1)*(R)*(C) + (c)*(R) + (r)
#define index3D(r, c, b, R, C, B) (R)*((b)*(C) + (c)) + (r)  //(b)*(R)*(C) + (c)*(R) + (r) 
#define index2D(r, c, R, C) (c)*(R) + (r)

//syntax: HIST_OUT = mexImgToIntHist(INT_HIST_IN, ROW_IN, COL_IN, BIN_IN, X1_IN, Y1_IN, X2_IN, Y2_IN);

void mexFunction(int nlhs, mxArray *plhs[], /* Output variables */ 
				 int nrhs, const mxArray *prhs[] /* Input variables */){
	
	//OUTPUT/INPUT PARAMETERS
	#define HIST_OUT plhs[0]
	#define INT_HIST_IN prhs[0]
	#define ROW_IN prhs[1]
	#define COL_IN prhs[2]
	#define BIN_IN prhs[3]
	#define X1_IN prhs[4]
	#define Y1_IN prhs[5]
	#define X2_IN prhs[6]
	#define Y2_IN prhs[7]
	
	//POINTER VARIABLES
	double *intHist, *hist;
	int i, j, k, x1, y1, x2, y2, Row, Col, Bin, histDims[3];

	//INPUT: Pointers
	intHist = mxGetPr(INT_HIST_IN); 
	Bin = (int)mxGetScalar(BIN_IN);
	Row = (int)mxGetScalar(ROW_IN);
	Col = (int)mxGetScalar(COL_IN);
	x1 = (int)mxGetScalar(X1_IN)-1;
	y1 = (int)mxGetScalar(Y1_IN)-1;
	x2 = (int)mxGetScalar(X2_IN)-1;
	y2 = (int)mxGetScalar(Y2_IN)-1;

	histDims[0] = Bin;
	histDims[1] = Bin;
	histDims[2] = Bin;
	
	//OUTPUT: Pointers
	HIST_OUT = mxCreateDoubleMatrix(Bin*Bin*Bin, 1, mxREAL);
	hist = mxGetPr(HIST_OUT);
	mxSetDimensions(HIST_OUT, histDims, 3);
	
	for(i=0; i<Bin; i++)
		for(j=0; j<Bin; j++)
			for(k=0; k<Bin; k++){
				hist[index3D(i,j,k,Bin,Bin,Bin)] = intHist[index5D(y2,x2,i,j,k,Row,Col,Bin,Bin,Bin)];
				if (y1-1 > 0) hist[index3D(i,j,k,Bin,Bin,Bin)] -= intHist[index5D(y1-1,x2,i,j,k,Row,Col,Bin,Bin,Bin)];
				if (x1-1 > 0) hist[index3D(i,j,k,Bin,Bin,Bin)] -= intHist[index5D(y2,x1-1,i,j,k,Row,Col,Bin,Bin,Bin)];
				if (y1-1 > 0 && x1-1 > 0) hist[index3D(i,j,k,Bin,Bin,Bin)] += intHist[index5D(y1-1,x1-1,i,j,k,Row,Col,Bin,Bin,Bin)];
			}
}