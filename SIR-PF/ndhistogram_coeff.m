% Both histograms must be normalised
% (i.e. bin values lie in range 0->1 and SUM(bins(i)) = 1
%       for i = {histogram1, histogram2} )

function [coef, normal] = ndhistogram_coeff(q,p) %,type
	%{
	if size(q) ~= size(p),
		coef = inf;
		normal = 0;
	end
	%}
	%% ----Bhattacharyya
	%if strcmp(type,'Bhattacharyya')==1
		normal = sum(sum(sum(sqrt(q).*sqrt(p))));
		
		% get the distance between the two distributions as follows
 		coef = sqrt(1 - normal);
		%return;
	%end
	
	%{
	%% ----Hellinger
	if strcmp(type,'Hellinger')==1
		coef = sum(sum(sum((sqrt(q) - sqrt(p)).^2)));
		
		% get the distance between the two distributions as follows
		normal = sqrt(1 - coef);
		return;
	end
	%}