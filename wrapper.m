% ***********************************************************************
% Copyright (c) Kristoffer Öfjäll, 2014, parts by Michael Felsberg, 2013, parts by Laura Sevilla-Lara and Erik G. Learned-Miller, 2012
% ***********************************************************************

function wrapper(q, learnrate, distanceFuncID, displayView)

if (nargin < 1)
    q = 4;
end

if (nargin < 2)
	learnrate = 0.05;
end

if (nargin < 3)
	distanceFuncID = 5;
end

if (nargin < 4)
    displayView = false;
end

%%
%% For copyright: see readme
%%

% *************************************************************
% VOT: Always call exit command at the end to terminate Matlab!
% *************************************************************

%if (~displayView)
 onCleanup(@() exit() );
%end

% *************************************************************
% VOT: Set random seed to a different value every time.
% *************************************************************
RandStream.setGlobalStream(RandStream('mt19937ar', 'Seed', sum(clock)));


% **********************************
% VOT: Read input data
% **********************************

[images, region] = vot_initialize();
region = round(region);
%% Initialize tracker variables

count = size(images,1);

im0 = imread(images{1});
height = size(im0,1);
width = size(im0,2);

results = zeros(count, 8);

results(1, :) = region;

xp = region(1:2:end);
yp = region(2:2:end);


x = round(min(xp));
y = round(min(yp));
W = round(max(xp)-x);
H = round(max(yp)-y);
if (x<1); x=1; end
if (y<1); y=1; end
if (x+W>width); x=width-W; end;
if (y+H>height); y=height-H; end;


T = imcrop(im0, [x y W H]);
% x = region(1);
% y = region(2);
% W = region(3);
% H = region(4);

%% set parameters for EDFT:
nbins = 15;
sp_width = [9, 15];
sp_sig = [1, 2];
max_shift = 30;
upd_factor = learnrate;

rng = [-4.53509 259.53509];
distanceParams.distanceFuncID = distanceFuncID;
switch(distanceFuncID)
    case 5
        [mu_ch, sigma2_ch] = getChannelMeanVar(nbins, rng);
        distanceDF = @(df1, df2) distanceDF5(df1, df2, mu_ch, sigma2_ch); % weighted L1 (normalized df)
        
    case 6
        [DW, G] = getCoherenceCalcObjects(nbins);
        distanceDF = @(df1, df2, wgt) distanceDF6(df1, df2, wgt); % coherence weighted
        distanceParams.DW = DW;
        distanceParams.G = G;
        
    otherwise
        error('No existing distance function selected');
end


last_pos = [y, x];
wsize = [H+1, W+1];

T = rgb2gray(T);
if (true)
    df = img2df(double(T), nbins, rng); % normal start df
else

end
    

for i=1:length(sp_width)
	target_models{i} = smoothDF(df, sp_width(i), sp_sig(i));
end;
last_motion = [0, 0];

% From 2nd frame to last one
for t=2:count
	f2 = rgb2gray(imread(images{t}));
	% compute new starting position using last motion
	init_pos = computeStartingPoint(last_pos, last_motion, wsize, size(f2));
    	% crop the image around the starting position for speed
    	crop_params = computeCropParams(last_pos, size(f2), wsize, max_shift, sp_width(end));
    	f2 = f2(crop_params(1):crop_params(1)+crop_params(3)-1, crop_params(2):crop_params(2)+crop_params(4)-1,:);
    	f2 = double(f2);
            
    	% find target in frame, map: distance map
    	%[pts, map] = findTargetHier(target_models, f2, init_pos-crop_params(1:2)+1, wsize, sp_width, sp_sig, nbins, rng, distanceDF, distanceParams); 
        pts = findTargetHier(target_models, f2, init_pos-crop_params(1:2)+1, wsize, sp_width, sp_sig, nbins, rng, distanceDF, distanceParams); 
    	end_pos = pts(end, :);
    
	% update target model 
    	f2 = f2(end_pos(1):end_pos(1)+wsize(1)-1, end_pos(2):end_pos(2)+wsize(2)-1,:);
    	df2 = img2df(f2, nbins, rng);
    	for j = 1:length(sp_width)
        	df2_s = smoothDF(df2, sp_width(j), sp_sig(j));
            if (q < 11000)
        	    target_models{j} = ( (1-upd_factor).*(target_models{j}).^q + upd_factor.*(df2_s).^q ).^(1/q);
            else
                target_models{j} = max( target_models{j},   df2_s );
            end
    	end;
    
	% store result
    	end_pos = end_pos+crop_params(1:2)-1;
    	x = end_pos(2);
	y = end_pos(1);
	last_motion = floor(0.5*(last_motion + end_pos - last_pos));
    	last_pos = end_pos; 
        
%     if (displayView)
%         displayTrackingResultAndModel(rgb2gray(imread(images{t})), [x, y, W, H], target_models, df2, nbins, rng, map);
%     end

	%results(t, :) = [x, y, W, H];
    results(t, :) = [x, y,  x+W, y,  x+W, y+H,  x, y+H];
end

vot_deinitialize(results);
