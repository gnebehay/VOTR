function [out] = gausssmooth2(in, sigma)
	x = [-ceil(3.0*sigma):ceil(3.0*sigma)];
	
	G = exp(-x.^2/(2*sigma^2))/(sqrt(2*pi)*sigma);
	G = G / sum(G);
	
	out = conv2(G, G, in, 'same');