function [hist] = intHistToHist3D(intHist, Bins, x1, y1, x2, y2)
	hist = zeros(Bins, 1);
	for j=1:Bins
		hist(j,1) = intHist(y2,x2,j);
		if (y1-1) > 0, hist(j,1) = hist(j,1) - intHist(y1-1,x2,j); end
		if (x1-1) > 0, hist(j,1) = hist(j,1) - intHist(y2,x1-1,j); end
		if ((y1-1) > 0) && ((x1-1) > 0), hist(j,1) = hist(j,1) + intHist(y1-1,x1-1,j); end
	end