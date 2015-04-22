% Generate channel centers and channel width parameters.

% ***********************************************************************
% Copyright (c) Kristoffer Öfjäll, 2014.
% ***********************************************************************


function [mu_ch, sigma2_ch] = getChannelMeanVar(nbins, rng)

% TODO: analytic expression for sigma2_ch

Nsamp = 1000;
chenc = ChannelEnc1D('cos2', 'exterior', nbins, rng);
B = chenc.basisMatrix(Nsamp);
x = linspace(rng(1),rng(2), Nsamp)';
dx = x(2)-x(1);
B = B./(sum(B(:,5))*dx);
mu_ch = chenc.centers;

% sigma bi
sigma2_ch = median(sum((repmat(x, [1 nbins]) - repmat(mu_ch, [Nsamp 1])).^2 .* B * dx));

