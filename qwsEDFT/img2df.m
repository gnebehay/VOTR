% ***********************************************************************
% Copyright (c) Michael Felsberg, 2013
% ***********************************************************************

function df = img2df(im, numBins,rng)

enc_o = ChannelEnc1D('cos2', 'exterior', numBins, rng);
df = encodeImg(enc_o, im);

