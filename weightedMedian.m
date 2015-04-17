function wMed = weightedMedian(D,W)

% ----------------------------------------------------------------------
% Function for calculating the weighted median 
%
% Input:    D ... matrix of observed values
%           W ... matrix of weights, W = ( w_ij )
% Output:   wMed ... weighted median                   
% ----------------------------------------------------------------------

% normalize the weights, such that: sum ( w_ij ) = 1
% (sum of all weights equal to one)

WSum = sum(W(:));
W = W / WSum;

% (line by line) transformation of the input-matrices to line-vectors 

% sort the vectors
A = [W' D'];
ASort = sortrows(A,2:size(A,2));

wSort = ASort(:,1)';
dSort = ASort(:,2:end)';

sumVec = cumsum(wSort);    % vector for cumulative sums of the weights

j = sum(sumVec < 0.5) + 1; %find(sumVec >= 0.5, 1, 'first');
wMed = [dSort(:,j)'];