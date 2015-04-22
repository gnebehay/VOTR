% Generate matrices for estimating coherence.

% ***********************************************************************
% Copyright (c) Kristoffer Öfjäll, 2014.
% ***********************************************************************


function [DW, G] = getCoherenceCalcObjects(nbins)



% for r1 and r2 --------------------
T = [[2 -1 -1]/sqrt(6)
     [0 1 -1]/sqrt(2)
     [1 1 1]/sqrt(3)]; % rotation 
G = 6*T'*[1 0 0;0 1 0;0 0 0]*T; % r2 distance in rotated space
DW = eye(nbins)+diag(ones(nbins-1,1),1)+diag(ones(nbins-1,1),-1); % decoding window matrices
DW = DW(2:end-1,:);
% ----------------------------------