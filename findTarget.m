% ***********************************************************************
% Copyright (c) Kristoffer Öfjäll, 2014, Laura Sevilla-Lara and Erik G. Learned-Miller, 2012.
% ***********************************************************************


%function [pt, map] = findTarget(df1, df2, pt, wsize, distanceDF, distanceParams)
function pt = findTarget(df1, df2, pt, wsize, distanceDF, distanceParams)
% Compute the flow of a patch

% Use a map of the distances to avoid repeated calculations 
map = -1*ones(size(df2, 1), size(df2, 2));


% weights (only depends on df1)
if (distanceParams.distanceFuncID == 6)
    df1 = df1 .* repmat( 1./ sum(df1,3), [1 1 size(df1,3)]); % normalize df1
    
    % sizes
    ds = size(df1);
    dfcmodel = reshape(df1, [ds(1)*ds(2) ds(3)]).'; % channels along columns
    Nfeat = ds(1)*ds(2);

    [~,dInd] = max(distanceParams.DW*dfcmodel); % decoding window start index
    CDW = dfcmodel(sub2ind(size(dfcmodel), [dInd;dInd+1;dInd+2], repmat(1:(Nfeat), [3 1]))); % all decoding windows
    r1 = zeros(1,Nfeat);
    for kk = 1:Nfeat
        r1(kk) = (CDW(:,kk).'*distanceParams.G*CDW(:,kk));
    end
    r2 = sum(dfcmodel).^2;
    coh = (r1./(r2+.00000000001)); % coherence
    coh = coh + 2;

    distFunc = @(dfcmp) distanceDF(dfcmodel, dfcmp, coh);
    
else
    distFunc = @(dfcmp) distanceDF(df1, dfcmp);
end



% compute starting distance 
start_dist = distFunc(df2(pt(1):pt(1)+wsize(1)-1, pt(2):pt(2)+wsize(2)-1, :));
map(pt(1), pt(2)) = start_dist;
min_dist = start_dist;
min_dist_idx = 0;

stop = 0;
shifts = [-1, 0; 0, 1; 1 0; 0 -1];

while ~stop
    
    % check the 4 neighbors
    for i=1:size(shifts, 1)
        
        dx = pt(1)+shifts(i, 1);
        dy = pt(2)+shifts(i, 2);
        
        try  
            % Compute distance if it hasn't been computed yet
            if map(dx, dy) == -1
                cur_dist = distFunc(df2(dx:dx+wsize(1)-1, dy:dy+wsize(2)-1, :));
                map(dx, dy) = cur_dist;
            else
                cur_dist = map(dx, dy);
            end;

            % Update best displacement if necessary
            if cur_dist < min_dist
                min_dist = cur_dist; 
                min_dist_idx = i;
            end;
            
        catch    
        end;      
            
    end;
    
    % move if it's better than the current position and doesn't exceed dimensions
    if min_dist_idx ~= 0 && min_dist < start_dist && ~isBorderPixel(size(df2), pt+shifts(min_dist_idx, :), wsize) 
        pt = pt + shifts(min_dist_idx, :);
        start_dist = min_dist;
    else
        stop = 1;
    end;

end;

        
        
