function [state, location] = tracker_pf_initialize(I, region, varargin)
	[height, width, d] = size(I);
	
	% ***************** %
	%     VARIABLES     %
	% ***************** %
	
	%REGION
	% If the provided region is a polygon ...
	if numel(region) > 4
		x1 = round(min(region(1:2:end)));
		x2 = round(max(region(1:2:end)));
		y1 = round(min(region(2:2:end)));
		y2 = round(max(region(2:2:end)));
		region = round([x1, y1, x2 - x1, y2 - y1]);
	else
		region = round([round(region(1)), round(region(2)), ... 
			round(region(1) + region(3)) - round(region(1)), ...
			round(region(2) + region(4)) - round(region(2))]);
	end;
	
	x1 		  = max(1, region(1));							% CENTER POINT x
	y1 		  = max(1, region(2));							% CENTER POINT y
	W 		  = region(3);									% Normal width
	H 		  = region(4);									% Normal height
	x2 		  = min(width, x1 + W)-1;
	y2 		  = min(height, y1 + H)-1;
	
	%Smaller width/height
	f_size	  = 0.3; %0.3
	sW 		  = ceil(W*f_size);								% Smaller width
	sH 		  = ceil(H*f_size);								% Smaller height
	fWH		  = [W-sW H-sH];
	
	%SMALLER REGION
	sx1 = x1+sW;
	sy1 = y1+sH;
	sx2 = x2-sW;
	sy2 = y2-sH;
	
	%BACKGROUND
	bPadd 	  = min(H, W)*0.25;											% Background padding
	%x1B 	  = round(max(1, x1 - bPadd));
	%y1B 	  = round(max(1, y1 - bPadd));
	%x2B 	  = round(min(width, x2 + bPadd));
	%y2B 	  = round(min(height, y2 + bPadd));
	
	%OTHER
	M 		  = round([x1 + x2, y1 + y2] / 2);				% MEDIAN
	M		  = [M W H];
	C 		  = [W/sqrt(3) 0; 0 H/sqrt(3)]; 			 	% COVARIANCE
	
	Step      = 16; %32
	Edges     = (1:Step:257)-1;
	Bins      = 256 / Step; %8
	
	N 		  = 300; 										% Number of samples
	w_pos 	  = 5;
	w_vec 	  = 2;
	w_size	  = 1;
	sigma 	  = 0.2;
	
	%NCV
	F = [1 0 1 0 0 0; ...			% position x
		 0 1 0 1 0 0; ...			% position y
		 0 0 1 0 0 0; ...			% velocity x
		 0 0 0 1 0 0; ...			% velocity y
		 0 0 0 0 1 0; ...			% width
		 0 0 0 0 0 1];				% height
	
	% **************** %
	% CREATE HISTOGRAM %
	% **************** %
	
	%img = double(I);
	img = double(rgb2ycbcr(I));
	img(:,:,1) 	= uint8(255 * ((img(:,:,1)-16)/(235-16)));
	img(:,:,2) 	= uint8(255 * ((img(:,:,2)-16)/(240-16)));
	img(:,:,3) 	= uint8(255 * ((img(:,:,3)-16)/(240-16)));

	%Darker images fix
	img(:,:,1)  	= imadjust(uint8(img(:,:,1)));
	%img(:,:,2)  	= imadjust(uint8(img(:,:,2)));
	%img(:,:,3)  	= imadjust(uint8(img(:,:,3)));
	
	T1 		 	 = double(img(sy1:sy2, sx1:sx2,:)); % Y Cb Cr tamplate
	%T1BG 	  	 = double(img(y1B:y2B, x1B:x2B,:));
	
	refHist   	 = ndhistogram(reshape(T1,[],d), Edges, Edges, Edges);
	refHist 	 = refHist./sum(refHist(:));
	v_u = nan;
	%{
	refBgHist 	 = ndhistogram(reshape(T1BG,[],d), Edges, Edges, Edges);
	refBgHist 	 = refBgHist - refHist;
	
	refHist = refHist./sum(refHist(:)); refBgHist = refBgHist./sum(refBgHist(:));
	
	[refHist, v_u] 	 = histN_BG(refHist, refBgHist);
	%}
	%INTEGRAL IMAGES
	%{
	HistY 	 	 = ndhistogram(reshape(T1(:,:,1),[],1), Edges); 
	HistCb 	 	 = ndhistogram(reshape(T1(:,:,2),[],1), Edges); 
	HistCr 	 	 = ndhistogram(reshape(T1(:,:,3),[],1), Edges); 
	
	%HistY = HistY./sum(HistY(:)); HistCb = HistCb./sum(HistCb(:)); HistCr = HistCr./sum(HistCr(:));
	
	HistYBG 	 = ndhistogram(reshape(T1BG(:,:,1),[],1), Edges); HistYBG  = HistYBG  - HistY;
	HistCbBG 	 = ndhistogram(reshape(T1BG(:,:,2),[],1), Edges); HistCbBG = HistCbBG - HistCb;
	HistCrBG 	 = ndhistogram(reshape(T1BG(:,:,3),[],1), Edges); HistCrBG = HistCrBG - HistCr;
	
	HistYBG = HistYBG./sum(HistYBG(:)); HistCbBG = HistCbBG./sum(HistCbBG(:)); HistCrBG = HistCrBG./sum(HistCrBG(:));
	HistY = HistY./sum(HistY(:)); HistCb = HistCb./sum(HistCb(:)); HistCr = HistCr./sum(HistCr(:));
	
	HistY 		 = histN_BG(HistY, 	HistYBG);
	HistCb 		 = histN_BG(HistCb, HistCbBG);
	HistCr 		 = histN_BG(HistCr, HistCrBG);
	%}
	% ****************** %
	% GENERATE PARTICLES %
	% ****************** %
	
	X = create_particles(M(1:2), C, N, fWH');
	%[HistY, HistCb, HistCr]
	state = struct('bPadd', bPadd, 'position', M, 'size', [W, H], ...
		'Ref_Hist', refHist, 'v_u', v_u, 'bins', Bins, 'edges', Edges,...
		'nParticles', N, 'Particles', X, 'mode', F, 'sigma', sigma, ...
		'w_pos', w_pos, 'w_vec', w_vec, 'w_size', w_size);
	
	state.window = max(state.size) * 2;
	
	%state
	
	location = [x1, y1, state.size];