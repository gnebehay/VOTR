function [state, location] = tracker_pf_update(state, I, varargin)
	[height, width, d] = size(I);
	
	% IF Target loss or out of bounds
	if (state.position(2) < 1 || state.position(2) > height || state.position(1) < 1 || state.position(1) > width), location = []; return; end;

	% UPDATE
	state.Particles = update_particles(state.Particles, state.mode, state.w_pos, state.w_vec, state.w_size);

	%FIX IF OUT OF IMAGE
	%state.Particles(1,(state.Particles(1,:)<1)) = M(1); state.Particles(1,(state.Particles(1,:)>width)) = M(1);
	%state.Particles(2,(state.Particles(2,:)<1)) = M(2); state.Particles(2,(state.Particles(2,:)>height)) = M(2);

	%img = double(I);
	img = double(rgb2ycbcr(I));
	img(:,:,1) 	= uint8(255 * ((img(:,:,1)-16)/(235-16)));
	img(:,:,2) 	= uint8(255 * ((img(:,:,2)-16)/(240-16)));
	img(:,:,3) 	= uint8(255 * ((img(:,:,3)-16)/(240-16)));

	%Darker images fix
	img(:,:,1)  	= imadjust(uint8(img(:,:,1)));
	%img(:,:,2)  	= imadjust(uint8(img(:,:,2)));
	%img(:,:,3)  	= imadjust(uint8(img(:,:,3)));
	
	img = double(img);
	
	%INTEGRAL IMAGES
	%{
	[~, ImgBins] 	= histc(uint8(img), state.edges);
	IntHistY 		= mexImgToIntHist3D(ImgBins(:,:,1), height, width, state.bins);
	IntHistCb 		= mexImgToIntHist3D(ImgBins(:,:,2), height, width, state.bins);
	IntHistCr 		= mexImgToIntHist3D(ImgBins(:,:,3), height, width, state.bins);
	%}
	
	% CALCULATE WEIGHTS
	%W = arrayfun(@(x,y,ww,wh) weights_hist_particles(height, width, d, state.bPadd, [ww wh], x, y, IntHistY, IntHistCb, IntHistCr, state.Ref_Hist, state.edges, state.bins), state.Particles(1,:), state.Particles(2,:), state.Particles(5,:), state.Particles(6,:));  % Speed up
	W = arrayfun(@(x,y,ww,wh) weights_hist_particles(img, height, width, d, [ww wh], x, y, state.Ref_Hist, state.v_u, state.edges, state.bins), state.Particles(1,:), state.Particles(2,:), state.Particles(5,:), state.Particles(6,:));  % Speed up
	
    W = exp(-0.5*W/state.sigma^2);
	
	state.position(1) = weightedMedian(state.Particles(1,:), W);
	state.position(2) = weightedMedian(state.Particles(2,:), W);
	%state.position(3:4) = weightedMedian(state.Particles(5:6,:), W);
	%[~,i] = max(W);
	%M(3:4) = state.Particles(5:6,i);
	
		
	% CORRECTION
	
	%REGION
	from = 		[min(width, max(1, floor(state.position(1) - state.position(3) / 2))), ...
				 min(height, max(1, floor(state.position(2) - state.position(4) / 2)))];
	to = 		[min(width, max(1, floor(state.position(1) + state.position(3) / 2)))-1, ...
				 min(height, max(1, floor(state.position(2) + state.position(4) / 2)))-1];
	%{
	%BACKGROUND
	fromB = 	[max(1, floor(from(1)-state.bPadd)), ...
				 max(1, floor(from(2)-state.bPadd))];
	toB = 		[min(width, floor(to(1)+state.bPadd)), ...
				 min(height, floor(to(2)+state.bPadd))];
	%}
	T2 			 = img(from(2):to(2), from(1):to(1),:);
	%T2BG 		 = img(fromB(2):toB(2), fromB(1):toB(1),:);
	
	Hist   		 = ndhistogram(reshape(T2,[],d), state.edges, state.edges, state.edges);
	Hist = Hist./sum(Hist(:));
	%{
	bgHist 	 	 = ndhistogram(reshape(T2BG,[],d), state.edges, state.edges, state.edges);
	bgHist 	 	 = bgHist - Hist;
	
	Hist = Hist./sum(Hist(:)); bgHist = bgHist./sum(bgHist(:));
	
	[Hist, t_v_u] 	 = histN_BG(Hist, bgHist);
	%}
	[~, tW] = ndhistogram_coeff(Hist,  state.Ref_Hist); %, 'Bhattacharyya'
	tW = exp(-0.5*((1-tW)^2)/state.sigma^2);
	
	if(tW >= 0.9 && tW <= 0.95)
		%fprintf('CORRECT\n');
		state.Ref_Hist = 0.05 * Hist  + (0.95) * state.Ref_Hist;
		%state.v_u = 0.05 * t_v_u + (0.95) * state.v_u;
	end;
	
	% RESAMPLE
	[state.Particles, ~] = resample_particles(state.Particles, W);

	%plot(state.position(1), state.position(2),'.','color', [1.0 0.0 0.0], 'MarkerSize', 10);

	location = [state.position(1:2) - ceil((state.position(3:4))/2 ), state.position(3:4)];