% Standard deviation weighted distance function. 
% Requires mu_ch and sigma2_ch parameters, see getChannelMeanVar and paper.

% ***********************************************************************
% Copyright (c) Kristoffer Öfjäll, 2014.
% ***********************************************************************


function dist = distanceDF5(dfcmodel, dfc2, mu_ch, sigma2_ch)

% weighted L1 (normalized)
dfcmodel = dfcmodel .* repmat( 1./ sum(dfcmodel,3), [1 1 size(dfcmodel,3)]);
dfc2 = dfc2 .* repmat( 1./ sum(dfc2,3), [1 1 size(dfc2,3)]);

ds = size(dfcmodel);

% pixel variance (model)
mu = mu_ch*(reshape(dfcmodel, [ds(1)*ds(2) ds(3)])');
sigma2 = sigma2_ch - mu.^2 + (mu_ch.^2) * (reshape(dfcmodel, [ds(1)*ds(2) ds(3)])');
sigma2 = reshape(sigma2, ds(1:2));
   

dfd = abs(dfcmodel-dfc2) ./ repmat(sqrt(sigma2), [1 1 ds(3)]);

dist = sum(dfd(:))/numel(dfcmodel);
    
    
    
