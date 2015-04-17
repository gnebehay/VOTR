%function [W, HistY, HistCb, HistCr] = weights_hist_particles(height, width, d, bPadd, w_size, X, Y, iY, iCb, iCr, refHist, edges, bins)	
function [W, Hist] = weights_hist_particles(I, height, width, d, w_size, X, Y, refHist, v_u, edges, bins)	
	
	if (X<1 || X > width || Y<1 || Y > height), 
		W = 1;
		Hist = zeros(bins,bins,bins);
		return; 
	end;
	
	%REGION
	from = 		[min(width, max(1, floor(X - w_size(1) / 2))), ...
				 min(height, max(1, floor(Y - w_size(2) / 2)))];
	to = 		[min(width, max(1, floor(X + w_size(1) / 2)))-1, ...
				 min(height, max(1, floor(Y + w_size(2) / 2)))-1];
	
	%BACKGROUND
	%{
	fromB = 	[max(1, floor(from(1)-bPadd)), ...
				 max(1, floor(from(2)-bPadd))];
	toB = 		[min(width, floor(to(1)+bPadd)), ...
				 min(height, floor(to(2)+bPadd))];
	%}
	
	T2 			 = I(from(2):to(2), from(1):to(1),:);
	%T2BG 		 = I(fromB(2):toB(2), fromB(1):toB(1),:);
	
	Hist   		 = ndhistogram(reshape(T2,[],d), edges, edges, edges);
	Hist 		 = Hist./sum(Hist(:));
	
	% transform hist by v_u (Dr. Comaniciu paper "Kernel based object tracking", PAMI2003
	%Hist = Hist .* v_u;
	%Hist = Hist./sum(Hist(:));
	
	[~, tW] = ndhistogram_coeff(Hist,  refHist); %, 'Bhattacharyya'
	
	W = (1-tW)^2;
	
	%{
	HistY 		= mexIntHistToHist3D(iY, height, width, bins, from(1), from(2), to(1), to(2));
	HistCb 		= mexIntHistToHist3D(iCb, height, width, bins, from(1), from(2), to(1), to(2));
	HistCr 		= mexIntHistToHist3D(iCr, height, width, bins, from(1), from(2), to(1), to(2));
	
	HistY = HistY./sum(HistY(:)); HistCb = HistCb./sum(HistCb(:)); HistCr = HistCr./sum(HistCr(:));
	
	HistYBG 	= mexIntHistToHist3D(iY, height, width, bins, fromB(1), fromB(2), toB(1), toB(2)); HistYBG  = HistYBG - HistY;
	HistCbBG 	= mexIntHistToHist3D(iCb, height, width, bins, fromB(1), fromB(2), toB(1), toB(2)); HistCbBG = HistCbBG - HistCb;
	HistCrBG 	= mexIntHistToHist3D(iCr, height, width, bins, fromB(1), fromB(2), toB(1), toB(2)); HistCrBG = HistCrBG - HistCr;
	
	HistYBG = HistYBG./sum(HistYBG(:)); HistCbBG = HistCbBG./sum(HistCbBG(:)); HistCrBG = HistCrBG./sum(HistCrBG(:));
	HistY = HistY./sum(HistY(:)); HistCb = HistCb./sum(HistCb(:)); HistCr = HistCr./sum(HistCr(:));
	
	HistY 		 = histN_BG(HistY, 	HistYBG);
	HistCb 		 = histN_BG(HistCb, HistCbBG);
	HistCr 		 = histN_BG(HistCr, HistCrBG);
	
	[~,  W_Y] = ndhistogram_coeff(HistY,  refHist(:,1), 'Bhattacharyya');
	[~, W_Cb] = ndhistogram_coeff(HistCb, refHist(:,2), 'Bhattacharyya');
	[~, W_Cr] = ndhistogram_coeff(HistCr, refHist(:,3), 'Bhattacharyya');
	
	W = (1-W_Y)^2 + (1-W_Cb)^2 + (1-W_Cr)^2;
	%}