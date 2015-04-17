function wrapper
%% VOT integration example wrapper (old approach)

% *************************************************************
% VOT: Always call exit command at the end to terminate Matlab!
% *************************************************************
cleanup = onCleanup(@() exit() );

% *************************************************************
% VOT: Set random seed to a different value every time.
% *************************************************************
RandStream.setGlobalStream(RandStream('mt19937ar', 'Seed', sum(clock)));

tracker = 'DSST';

% **********************************
% VOT: Read input data
% **********************************
[images, region] = vot_tracker_initialize();

results = cell(length(images), 1);

padding = 1.0;
output_sigma_factor = 1/16;
scale_sigma_factor = 1/4;
lambda = 1e-2;
interp_factor = 0.025;	
features = {'gray','FHOG1'};

nScales = 33;
scale_model_factor = 1.0;
scale_step = 1.02;
scale_model_max_area = 32*16;
scale_feature = 'HOG4';

% If the provided region is a polygon ...
if numel(region) > 4
    % Init with an axis aligned bounding box with correct area and center
    % coordinate
    cx = mean(region(1:2:end));
    cy = mean(region(2:2:end));
    x1 = min(region(1:2:end));
    x2 = max(region(1:2:end));
    y1 = min(region(2:2:end));
    y2 = max(region(2:2:end));
    A1 = norm(region(1:2) - region(3:4)) * norm(region(3:4) - region(5:6));
    A2 = (x2 - x1) * (y2 - y1);
    s = sqrt(A1/A2);
    w = s * (x2 - x1) + 1;
    h = s * (y2 - y1) + 1;
else
    cx = region(1) + (region(3) - 1)/2;
    cy = region(2) + (region(4) - 1)/2;
    w = region(3);
    h = region(4);
end

pos = round([cy cx]);
target_sz = round([h w]);

num_frames = numel(images);

%notation: variables ending with f are in the frequency domain.

init_target_sz = target_sz;
currentScaleFactor = 1.0;

% target size att scale = 1
base_target_sz = target_sz / currentScaleFactor;

%window size, taking padding into account
sz = floor(base_target_sz * (1 + padding));

%desired output (gaussian shaped), bandwidth proportional to target size
output_sigma = sqrt(prod(base_target_sz)) * output_sigma_factor;
scale_sigma = sqrt(nScales) * scale_sigma_factor;
[rs, cs] = ndgrid((1:sz(1)) - floor(sz(1)/2), (1:sz(2)) - floor(sz(2)/2));

y = exp(-0.5 * (((rs.^2 + cs.^2) / output_sigma^2)));
yf = single(fft2(y));


%label function for the scales
ss = (1:nScales) - ceil(nScales/2);
ys = exp(-0.5 * (ss.^2) / scale_sigma^2);
ysf = single(fft(ys));

%store pre-computed cosine window
cos_window = single(hann(sz(1)) * hann(sz(2))');

if mod(nScales,2) == 0
    scale_window = single(hann(nScales+1));
    scale_window = scale_window(2:end);
else
    scale_window = single(hann(nScales));
end;

ss = 1:nScales;
scaleFactors = scale_step.^(ceil(nScales/2) - ss);

if scale_model_factor^2 * prod(init_target_sz) > scale_model_max_area
    scale_model_factor = sqrt(scale_model_max_area/prod(init_target_sz));
end

scale_model_sz = floor(init_target_sz * scale_model_factor);


scaleSizeFactors = scaleFactors;%/max(scaleFactors);

% find maximum and minimum scales
im = imread(images{1});
min_scale_factor = scale_step ^ ceil(log(max(5 ./ sz)) / log(scale_step));
max_scale_factor = scale_step ^ floor(log(min([size(im,1) size(im,2)] ./ base_target_sz)) / log(scale_step));

for frame = 1:num_frames,
    %load image
    im = imread(images{frame});
    
    if frame > 1
        xt = get_subwindow(im, pos, sz, currentScaleFactor, cos_window, features);
        xtf = fft2(xt);
        response = real(ifft2(sum(hf_num .* xtf, 3) ./ (hf_den + lambda) ));
        
        [row, col] = ind2sub(size(response),find(response == max(response(:)), 1));
        
        %target location is at the maximum response
        pos = pos + round((-sz/2 + [row, col]) * currentScaleFactor);
        
        %do a scale space search aswell
        xs = get_scale_subwindow(im, pos, base_target_sz, currentScaleFactor * scaleSizeFactors, scale_window, scale_model_sz, scale_feature);
        xsf = fft(xs,[],2);
        
        scale_response = real(ifft(sum(sf_num .* xsf, 1) ./ (sf_den + lambda) ));
        
        recovered_scale = ind2sub(size(scale_response),find(scale_response == max(scale_response(:)), 1));
        
        %set the scale
        currentScaleFactor = currentScaleFactor * scaleFactors(recovered_scale);
        
        if currentScaleFactor < min_scale_factor
            currentScaleFactor = min_scale_factor;
        elseif currentScaleFactor > max_scale_factor
            currentScaleFactor = max_scale_factor;
        end
    end
    
    xl = get_subwindow(im, pos, sz, currentScaleFactor, cos_window, features);
    
    xlf = fft2(xl);
    
    new_hf_num = bsxfun(@times, yf, conj(xlf));
    new_hf_den = sum(xlf .* conj(xlf), 3);
    
    %make a scale search model aswell
    xs = get_scale_subwindow(im, pos, base_target_sz, currentScaleFactor * scaleSizeFactors, scale_window, scale_model_sz, scale_feature);
    
    %fft over the scale dim
    xsf = fft(xs,[],2);
    new_sf_num = bsxfun(@times, ysf, conj(xsf));
    new_sf_den = sum(xsf .* conj(xsf), 1);
    
    
    if frame == 1,  %first frame, train with a single image
        hf_den = new_hf_den;
        hf_num = new_hf_num;
        sf_den = new_sf_den;
        sf_num = new_sf_num;
    else
        hf_den = (1 - interp_factor) * hf_den + interp_factor * new_hf_den;
        hf_num = (1 - interp_factor) * hf_num + interp_factor * new_hf_num;
        
        sf_den = (1 - interp_factor) * sf_den + interp_factor * new_sf_den;
        sf_num = (1 - interp_factor) * sf_num + interp_factor * new_sf_num;
    end
    
    target_sz = floor(base_target_sz * currentScaleFactor);
    
    %save position
    location = [pos([2,1]) - floor(target_sz([2,1])/2), target_sz([2,1])];
    
    if isempty(location)
        location = 0;
    end;
    
    results{frame} = location;
end

% **********************************
% VOT: Output the results
% **********************************
vot_tracker_results(results);

