%
%  M = basisMatrix(obj, nsamps)
%
%  Returns a matrix M, where each column is a sampled basis function using
%  'nsamps' samples equally spaced within the bounds. 
%
%  A sampled pdf 'p' can then be channel encoded by ch = M'*p. The basis 
%  matrix can also be used for easy visualization, MEM reconstruction etc.
%
