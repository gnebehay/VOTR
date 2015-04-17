% function qHistogram = ndHistc (mData, vEdge1, vEdge2, ... )
%
% * Input Arguments: 
%
%      + mData: nRecord by nDim 2-dimensional array of doubles
%      + vEdge1, vEdge2, ... : nDim vectors of histogram edges
%
% * Return value:
%
%      + qHistogram: nDim-dimensional data cube 
%           containing number of points in each cell 
%           defined by histogram edges. For instance, 
%           qHistogram(1,1,1,...) means number of data points
%           satisfying 
%               vEdge1(1) <= mData(:,1) < vEdge1(2) & ...
%               vEdge2(1) <= mData(:,2) < vEdge2(1) & ...
%               vEdge3(1) <= mData(:,3) < vEdge3(1) & ...
%               ...
% * Example
%       mRand = rand(1e6,5);
%       ve1 = linspace(0,1,5);
%       ve2 = linspace(0,1,6);
%       ve3 = linspace(0,1,7);
%       ve4 = linspace(0,1,8);
%       ve5 = linspace(0,1,9);
%       qHist = ndhistc(mRand, ve1, ve2, ve3, ve4, ve5);
%      
% * Comparison with ndhist.m (compiled using mcc -x ndhist)
%      + 1e6 by 2 data -> 5 by 6
%          ndhist.m    79.49   sec
%          ndhistc.c    0.4610 sec
%
%      + 1e6 by 5 data -> 5 by 6 by 7 by 8 by 9
%          ndhist.m   199.32   sec
%          ndhistc.c    2.4430 sec
%
%      ==> More efficient if More data points & Less dimensions
%

%   CopyLEFT (c) 2003 by Kangwon "Wayne" Lee. ;)
%   $Revision: 1.0 $
%   Implemented in a MATLAB mex file.
%#mex