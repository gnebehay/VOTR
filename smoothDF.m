% ***********************************************************************
% Copyright (c) Michael Felsberg, 2013
% ***********************************************************************

function df = smoothDF(df, size_kernel, sig_kernel)

h = fspecial('gaussian', size_kernel(1), sig_kernel(1));

% convolve in space 
for i=1:size(df, 3)
	df(:,:,i) = conv2(df(:,:,i), h, 'same');  
end
