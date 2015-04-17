function [histR, v_u] = histN_BG(hist, histBG)
	%warning('off','MATLAB:rankDeficientMatrix');
	
	v_u = histBG;
	
	% by Dr. Comaniciu etal's PAMI2003 paper
	v_u(~v_u) = nan;
	v_u = min(v_u(:))./v_u;
	v_u(isnan(v_u)) = 1;
	
	histR = hist.*v_u/sum(hist(:).*v_u(:)); %FINAL HIST