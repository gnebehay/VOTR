function [X, W] = resample_particles(X, W)
	W = W/sum(W);
	
	% Calculating Cumulative Distribution
	
	R = cumsum(W)';
	R = R./max(R);

	% Generating Random Numbers

	N = size(X, 2);
	%T = linspace(eps,1-eps,N);
	T = linspace(0+(1/N)/2,1-(1/N)/2,N);
	%T = rand(1,N);
	
	% Resampling

		%First option
	%I = arrayfun(@(x) find(R >= x,1,'first'), T );

		%Second option
	[~, I] = histc(T, R);
	I=I+1;

	X = X(:, I);
    W = W(I) ;