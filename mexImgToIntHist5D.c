#include "mex.h"

#define index5D(r, c, b1, b2, b3, R, C, B1, B2, B3) (R)*((C)*((B1)*((b3)*(B2) + (b2)) + (b1)) + (c)) + (r)  //(b3)*(R)*(C)*(B1)*(B2) + (b2)*(R)*(C)*(B1) + (b1)*(R)*(C) + (c)*(R) + (r)
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
	int r, c, b1, b2, b3, Row, Col, Bin, intHistDims[5];

	//INPUT: Pointers
	imgBins = mxGetPr(IMG_IN);    
	Row = (int)mxGetScalar(ROW_IN);
	Col = (int)mxGetScalar(COL_IN);
	Bin = (int)mxGetScalar(BIN_IN);

	intHistDims[0] = Row;
	intHistDims[1] = Col;
	intHistDims[2] = Bin;
	intHistDims[3] = Bin;
	intHistDims[4] = Bin;
	
	//OUTPUT: Pointers
	HIST_OUT = mxCreateDoubleMatrix(Row*Col*Bin*Bin*Bin, 1, mxREAL);
	intHist = mxGetPr(HIST_OUT);
	mxSetDimensions(HIST_OUT, intHistDims, 5);
	//memset(intHist, 0, sizeof(double)*Row*Col*Bin*Bin*Bin);
	
	/* IMAGE INTEGRAL HISTOGRAM */
	
	for(r=0; r<Row; r++){
		for(c=0; c<Col; c++){
			//INITIALIZE
			for(b1=0; b1<Bin; b1++) for(b2=0; b2<Bin; b2++) for(b3=0; b3<Bin; b3++) intHist[index5D(r,c,b1,b2,b3,Row,Col,Bin,Bin,Bin)] = 0;
			
			//CHECK IF IMG_IN_BINs == BIN_IN
			b1 = imgBins[index3D(r,c,0,Row,Col,3)];
			b2 = imgBins[index3D(r,c,1,Row,Col,3)];
			b3 = imgBins[index3D(r,c,2,Row,Col,3)];
			if (b1 > Bin || b2 > Bin || b3 > Bin) mexErrMsgTxt("IMG_IN Beans exceed BIN_IN");
			
			intHist[index5D(r,c,b1-1,b2-1,b3-1,Row,Col,Bin,Bin,Bin)] = 1;
			for(b1=0; b1<Bin; b1++) for(b2=0; b2<Bin; b2++) for(b3=0; b3<Bin; b3++){
				if (r > 0) 			intHist[index5D(r,c,b1,b2,b3,Row,Col,Bin,Bin,Bin)] += intHist[index5D(r-1,c  ,b1,b2,b3,Row,Col,Bin,Bin,Bin)];
				if (c > 0) 			intHist[index5D(r,c,b1,b2,b3,Row,Col,Bin,Bin,Bin)] += intHist[index5D(r  ,c-1,b1,b2,b3,Row,Col,Bin,Bin,Bin)];
				if (r > 0 && c > 0) intHist[index5D(r,c,b1,b2,b3,Row,Col,Bin,Bin,Bin)] -= intHist[index5D(r-1,c-1,b1,b2,b3,Row,Col,Bin,Bin,Bin)];
			}
		}
	}
}