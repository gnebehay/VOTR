% ***********************************************************************
% Copyright (c) Kristoffer Öfjäll, 2014, Laura Sevilla-Lara and Erik G. Learned-Miller, 2012, Michael Felsberg, 2013
% ***********************************************************************

%function [pts, map] = findTargetHier(target_models, f2, init_pos, wsize, sp_width, sp_sig, nbins, rng, distanceDF, distanceParams)
function pts = findTargetHier(target_models, f2, init_pos, wsize, sp_width, sp_sig, nbins, rng, distanceDF, distanceParams)

% it returns multiple pts for analysis purpose

% from coarse to fine kernel
cur_pt = init_pos;
%pts = cur_pt;
df2 = img2df(double(f2), nbins, rng);

for i=length(sp_width):-1:1
    % explode and convolve 
    df2_s = smoothDF(df2, sp_width(i), sp_sig(i));
    % find target
    %if (i == length(sp_width)) % save first map (coarsest level)
     %   [cur_pt, map] = findTarget(target_models{i}, df2_s, cur_pt, wsize, distanceDF, distanceParams); 
    %else
        cur_pt = findTarget(target_models{i}, df2_s, cur_pt, wsize, distanceDF, distanceParams); 
    %end
    %pts = [pts; cur_pt];
end;
pts = cur_pt;



