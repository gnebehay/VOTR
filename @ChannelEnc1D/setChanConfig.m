%ChannelEnc1D/setChanConfig
%
%  Sets the number of channels, width and position in input space. Some
%  different modes are available for defining the channel positions
%
%  obj = setChanConfig(obj, 'manual', nch, ssc, fpos, cscale, mfl)
%    The most basic method, similar to the cos^2 implementation by PEF.
%    Places the channels manually according to the parameters
%      nch:     Number of channels
%      ssc:     Spatial spacing of channels
%      fpos:    Position of the first channel
%      cscale:  Scaling of the basis function (optional, default==1)
%      mfl:     Modular flag (optional, default==0)
%      bounds:  Bounds of the input space. Used only for encodeDensity and
%               basisMatrix. (optional, default = first and last channel center)
%
%    Note that the 'cscale' increases/decreases the overlap between
%    channels. The channel width is always scaled according to ssc.
%      
%  cf = setChanConfig(obj, 'exterior', nch, bounds, cscale, mfl)
%    Places and scales the channels such that 'k' channels are active at 
%    all positions within the range specified by 'bounds' (length-2 vector). 
%    'k' is deduced from the channel width, e.g. standard cos2/B-splines 
%    with width 1.5 ==> k=3. The other parameters are as before.
%
%  cf = ChannelEnc1D(obj, 'interior', nch, bounds, cscale, mfl)
%    Places and scales the channels such that the first and last
%    basis function starts/ends exactly at the bounds. This means that no
%    basis functions are active outside of the bounds. 
%
