%  [img1, conf1, img2, conf2, ...] = decodeImg(obj, chimg, nmodes)
% 
%  Decodes a channel encoded image. Parameters:
%    chimg:  Channel coded image, as returned by 'encodeImage'. chimg(x,y,:)
%            is the channel encoded value of pixel (x,y).
%    obj:    ChannelEnc1D object
%    nmodes: Number of modes to return for each pixel (default==1)
%
%  Returns 'nmodes' decoded images with associated confidences.
%  Ex:
%    chimg = encodeImg(enc, img);
%    [img2, conf] = decodeImg(enc, chimg);
%
%  [CVL 2004-2007 (see README.txt for proper credits)]
