function [hist] = intHistToHist5D(intHist, Bins, x1, y1, x2, y2)
	hist = zeros(Bins, Bins, Bins);
	for i=1:Bins
		for j=1:Bins
			for k=1:Bins
				hist(i,j,k) = intHist(y2,x2,i,j,k);
				if (y1-1) > 0, hist(i,j,k) = hist(i,j,k) - intHist(y1-1,x2,i,j,k); end
				if (x1-1) > 0, hist(i,j,k) = hist(i,j,k) - intHist(y2,x1-1,i,j,k); end
				if ((y1-1) > 0) && ((x1-1) > 0), hist(i,j,k) = hist(i,j,k) + intHist(y1-1,x1-1,i,j,k); end
			end
		end
	end