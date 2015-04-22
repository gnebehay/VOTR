%ChannelEnc1D/encodeDensity
%
%  ch = encodeDensity(obj, pdf)
%
%  Encodes the density 'pdf', i.e. computes
%    c_i = <pdf, K_i>
%  where K_i is the i'th channel basis function. In other words, 
%  projects 'pdf' on the subspace spanned by {K_i}.
%
%  If sum(pdf) = 1, then ch = E[mean(enc(x))]
%
