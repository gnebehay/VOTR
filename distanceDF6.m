% Coherence weighted distance function. 
% Requires wgt, pre-calculated weights, see findTarget.m, 
% getCoherenceCalcObjects.m and paper.

% ***********************************************************************
% Copyright (c) Kristoffer Öfjäll, 2014.
% ***********************************************************************

function dist = distanceDF6(dfcmodel, dfc2, wgt)

% coherence weighted


% L1 normalization (global!)
% already normalized: dfcmodel = dfcmodel .* repmat( 1./ sum(dfcmodel,3), [1 1 size(dfcmodel,3)]);
dfc2 = dfc2 .* repmat( 1./ sum(dfc2,3), [1 1 size(dfc2,3)]);

% sizes
ds = size(dfcmodel);
%dfcmodel = reshape(dfcmodel, [ds(1)*ds(2) ds(3)]).'; % channels along columns
dfc2 = reshape(dfc2, [ds(2) ds(1)]).';
%Nfeat = ds(1)*ds(2);

% weight calculation: for reference
% decoding window
% [~,dInd] = max(DW*dfcmodel); % decoding window start index
% CDW = dfcmodel(sub2ind(size(dfcmodel), [dInd;dInd+1;dInd+2], repmat(1:(Nfeat), [3 1]))); % all decoding windows
% r1 = zeros(1,Nfeat);
% for kk = 1:Nfeat
%     r1(kk) = sqrt(CDW(:,kk).'*G*CDW(:,kk));
% end
% r2 = sum(dfcmodel);

 

% weighted difference
dfd = abs(dfcmodel-dfc2) .* repmat(wgt, [ds(1), 1]);

dist = sum(dfd(:))/numel(dfcmodel);

    
