#include "mex.h"

#define index3D(r, c, b, R, C, B) (R)*((b)*(C) + (c)) + (r)  //(b)*(R)*(C) + (c)*(R) + (r) 
#define index2D(r, c, R, C) (c)*(R) + (r)

//syntax: HIST_OUT = mexImgToIntHist(IMG_IN, ROW_IN, COL_IN, BIN_IN);

void mexFunction(int nlhs, mxArray *plhs[], /* Output variables */ 
				 int nrhs, const mxArray *prhs[] /* Input variables */){
	
	//OUTPUT/INPUT PARAMETERS
	#define HIST_OUT plhs[0]
	#define IMG_IN prhs[0]
	#define ROW_IN prhs[1]
	#define COL_IN prhs[2]
	#define BIN_IN prhs[3]
	
	//POINTER VARIABLES
	double *imgBins, *intHist;
	int r, c, b, Row, Col, Bin, intHistDims[3];
	
	//INPUT: Pointers
	imgBins = mxGetPr(IMG_IN);    
	Row = (int)mxGetScalar(ROW_IN);
	Col = (int)mxGetScalar(COL_IN);
	Bin = (int)mxGetScalar(BIN_IN);
	
	intHistDims[0] = Row;
	intHistDims[1] = Col;
	intHistDims[2] = Bin;
	
	//OUTPUT: Pointers
	HIST_OUT = mxCreateDoubleMatrix(Row*Col*Bin, 1, mxREAL);
	intHist = mxGetPr(HIST_OUT);
	mxSetDimensions(HIST_OUT, intHistDims, 3);
	//memset(intHist, 0, sizeof(double)*Row*Col*Bin);
	
	/* IMAGE INTEGRAL HISTOGRAM */
	
	for(r=0; r<Row; r++){
		for(c=0; c<Col; c++){
			//INITIALIZE
			for(b=0; b<Bin; b++) intHist[index3D(r,c,b,Row,Col,Bin)] = 0;
			
			//CHECK IF IMG_IN_BINs == BIN_IN
			b = imgBins[index2D(r,c,Row,Col)];
			if (b > Bin) mexErrMsgTxt("IMG_IN Beans exceed BIN_IN");
			
			intHist[index3D(r,c,b-1,Row,Col,Bin)] = 1;
			for(b=0; b<Bin; b++){
				if (r > 0) 			intHist[index3D(r,c,b,Row,Col,Bin)] += intHist[index3D(r-1,c  ,b,Row,Col,Bin)];
				if (c > 0) 			intHist[index3D(r,c,b,Row,Col,Bin)] += intHist[index3D(r  ,c-1,b,Row,Col,Bin)];
				if (r > 0 && c > 0) intHist[index3D(r,c,b,Row,Col,Bin)] -= intHist[index3D(r-1,c-1,b,Row,Col,Bin)];
			}
		}
	}
}