classdef ChannelEnc1D
%
%  ChannelEnc1D
%
%  Class for 1D channel encoding. Contains all channel encoding
%  parameters, including number of channels, basis function etc.
%  Offers different modes of configuring the representation:
% 
%  obj = ChannelEnc1D(basisfuncname, ...)
%    Creates a Channel encoding object using one of the standard basis 
%    functions. The following options are available:
%      'cos2'    cos^2 function
%      'bsp1'    First order b-spline
%      'bsp2'    Second order b-spline
%      'rect'    Rectangular function (producing a regular histogram)
%      'pchan'   P-channel basis (rectangular+linear offset)
%      'rectlin' (same as 'pchan')
%    A suitable decoding procedure is also selected.
%
%  obj = ChannelEnc1D(basisfunc, bfuncwidth, decfunc, ...)
%    Uses a custom basis function. Parameters:
%      basisfunc:  Function handle to a basis function following the same
%                  interface as the standard kernels 'bsp2kernel', 
%                  'pkernel' etc. 
%      bfuncwidth: The width of the basis function, such that 
%                  basisfunc(x)=0 for abs(x)>bfuncwidth.
%      decfunc:    Decoding function, following the same interface as
%                  the standard decoding functions (see ChannelEnc1D/decode)
%                  (optional)
%
%  After the basis function parameters, additional channel configuration
%  parameters can be specified, which get sent directly to setChanConfig
%  (type 'help setChanConfig' for more info)
%
%  Member functions:
%    setChanConfig
%    centers
%    encode
%    encodeImg
%    encodeDensity
%    decode
%    decodeImg
%    basisMatrix
%
%
%  Ex1: (basic functionality)
%    > enc = ChannelEnc1D('bsp2', 'exterior', 8, [0 1]);
%    > ch = encode(enc, 0.7);
%    > val = decode(enc, ch);
%
%  Ex2: (custom decoder function)
%    > enc = ChannelEnc1D(@bsp2kernel, 1.5, @mydecoder, 'exterior', 8, [0 1]);
%
%  Ex3: (custom kernel with larger overlap, no decoder)
%    > enc = ChannelEnc1D(@mykernel, 2.0, 'manual', 8, 1, 1);
%
%  [CVL 2004-2007 (see README.txt for credits)]

properties (SetAccess = 'public', GetAccess = 'public')
  
  % Encoding and decoding functions, followin the standard calling
  % convention
  kernelf;
  decf;

  % Basic channel parameters
  nchans = [];  % number of channels
  bounds = [];  % Bounds (lower, upper)
  fpos = [];    % Position of first channel
  ssc = [];     % Spatial spacing 
  mflag = [];   % Modular of not
  cscale = 1;   % Input space scaling of basis functions
  
  % Width of the basis function. Set by this class if any of the
  % default funtions are used, otherwise by the user.
  bfuncwidth = [];
end
  
  
methods

  % --- Constructor and configuration -------------------------------------
  
  % Constructor. Accepts parameters for which basis function to use.
  % After the constructor is done, the members 'encf', 'bfuncwidth', and
  % (optionally) 'bfuncwidth' are set.
  function obj = ChannelEnc1D(varargin)

    enc = varargin{1};
    switch class(enc)
      
      case 'char'
        switch enc
          case 'cos2'
            obj.kernelf = @cos2kernel;
            obj.decf = @cos2decode;
            obj.bfuncwidth = 1.5;
          case 'bsp1'
            obj.kernelf = @bsp1kernel;
            obj.decf = @bsp1decode;
            obj.bfuncwidth = 1.0;
          case 'bsp2'
            obj.kernelf = @bsp2kernel;
            obj.decf = @bsp2decode;
            obj.bfuncwidth = 1.5;
          case 'rect'
            obj.kernelf = @rectkernel; % Useful for comparison with hard-binned methods
            obj.decf = @rectdecode;
            obj.bfuncwidth = 0.5;
          case {'rectlin', 'pchan'}
            obj.kernelf = @pkernel;
            obj.decf = @pdecode;
            obj.bfuncwidth = 0.5;
          otherwise
            error('Unknown encoding');
        end
        
        restargs = varargin(2:end); % Remove the first argument
        
        
      case 'function_handle'
        if length(varargin)>=2
          obj.kernelf = enc;
          obj.bfuncwidth = varargin{2};
        else
          error('Must specify bfuncwidth when using custom basis functions');
        end
        restargs = varargin(3:end); % Remove the first two arguments

        
      otherwise
        error('Invalid call to constructor');
        
    end

    % There are more arguments. First, look for a decoding function
    if not(isempty(restargs))
      if isa(restargs{1}, 'function_handle')
        obj.decf = restargs{1};
        restargs = restargs(2:end);
      end
      
      if not(isempty(restargs))
        % There are even more arguments after the decoding function. Pass
        % on to setChanConfig
        obj = setChanConfig(obj, restargs{:});
      end
    end

    
  end
  
  
  %  Sets the channel configuration (number of channels, positions etc)
  function obj = setChanConfig(obj, mode, varargin)
    
    switch mode
      case 'manual'
      % enc = setChanLayout(enc, 'manual', nch, ssc, fpos, cscale, mflag)
      
        if length(varargin)<3, error('Too few input arguments!'); end;
      
        % Set default values
        if length(varargin)<4 || isempty(varargin{4}), varargin{4} = 1; end;
        if length(varargin)<5 || isempty(varargin{5}), varargin{5} = 0; end;
      
        obj.nchans = varargin{1};
        obj.ssc = varargin{2};
        obj.fpos = varargin{3};
        obj.cscale = varargin{4};
        obj.mflag = varargin{5};

        % Bounds
        if length(varargin)<6 || isempty(varargin{6})
          obj.bounds = [obj.fpos, obj.fpos + (obj.nchans-1)*obj.ssc];
        else
          obj.bounds = varargin{6};
        end

        
      case 'exterior'
      % enc = obj.setChanLayout(enc, 'exterior', nch, bounds, cscale, mflag)

        if length(varargin)<2, error('Too few input arguments!'); end;
      
        % Set default values
        if length(varargin)<3 || isempty(varargin{3}), varargin{3} = 1; end;
        if length(varargin)<4 || isempty(varargin{4}), varargin{4} = 0; end;

        obj.nchans = varargin{1};
        obj.bounds = varargin{2};
        obj.cscale = varargin{3};
        obj.mflag = varargin{4};

        if not(obj.mflag)
          % Linear domain
          d = obj.cscale*obj.bfuncwidth;
          obj.fpos = (obj.bounds(2)+obj.bounds(1)*obj.nchans-d*(obj.bounds(1)+obj.bounds(2))) / (obj.nchans+1-2*d);

          obj.ssc = (obj.bounds(2)-obj.bounds(1)) / (obj.nchans+1-2*d);
        else
          % Modular domain 
          % (note that exterior==interior spacing for modular domanis)
          obj.fpos = obj.bounds(1);
          obj.ssc = (obj.bounds(2)-obj.bounds(1)) / obj.nchans;
        end
        
      
      case 'interior'
      % enc = setChanLayout(enc, 'interior', nch, bounds, cscale, mflag)

        if length(varargin)<2, error('Too few input arguments!'); end;

      
        % Set default values
        if length(varargin)<3 || isempty(varargin{3}), varargin{3} = 1; end;
        if length(varargin)<4 || isempty(varargin{4}), varargin{4} = 0; end;

        obj.nchans = varargin{1};
        obj.bounds = varargin{2};
        obj.cscale = varargin{3};
        obj.mflag = varargin{4};

        % Compute the other parameters
        if not(obj.mflag)
          d = obj.cscale*obj.bfuncwidth;
          obj.ssc = (obj.bounds(2)-obj.bounds(1))/(obj.nchans-1+2*d);
          obj.fpos = obj.bounds(1) + d*obj.ssc;
        else
          % Modular domain
          obj.fpos = obj.bounds(1);
          obj.ssc = (obj.bounds(2)-obj.bounds(1)) / obj.nchans;
        end
        
    end
    
  end

  
  % --- Encoding and decoding ---------------------------------------------
  
  function ch = encode(obj, val, chi)
  
    % Error checking
    [sr, sc] = size(val);
    if sr~=1
      error('val has to be a scalar or a row vector');
    end
    
    if not(isa(val, 'double'))
      error('Only encoding of doubles are supported');
    end

    % Normalize val such that we can use channels at (1,2,3,...)
    val = (val-obj.fpos)/obj.ssc + 1;
      
    if nargin<3
      cpos = [1:obj.nchans]';  % Unit channel positions
    else
      cpos = chi; % A single channel
    end
    
    i1 = ones(size(val));
    i2 = ones(size(cpos));
    
    % Calculate normalized spatial distance
    if obj.mflag
      % Old version. Not correct, since we can no longer assume symmetric
      % basis functions.
      % ndist = obj.nchans/2 - abs(mod(cpos*i1-i2*val, obj.nchans) - obj.nchans/2);

      % New version (Erik 2007). Works, but is a bit clumpsy..
      slask = i2*val-cpos*i1;
      slasklin = slask(:);
      M = [slasklin, slasklin-obj.nchans, slasklin+obj.nchans];
      [val, ix] = min(abs(M), [], 2);
      ix = sub2ind(size(M), [1:size(M,1)]', ix);
      ndist = reshape(M(ix), size(slask));
    else
      ndist = i2*val-cpos*i1;
    end
    
    ch = feval(obj.kernelf, ndist/obj.cscale);
  end


  function chimg = encodeImg(obj, img)
    % Rehape the image to a row vector, encode each pixel, and reshape back
    sz = size(img);
    chimg = encode(obj, img(:)');
    chimg = reshape(chimg', [sz, obj.nchans]);
  end
  
  
  function ch = encodeDensity(cf, pdf)
    ch = basisMatrix(cf, length(pdf))' * pdf(:);
  end


  function [vals, cert] = decode(obj, ch, nmodes)
    if nargin<3, nmodes = 1; end;
    
    if isempty(obj.decf)
      error('No decoding method specified');
    else
        %Decoding done separately for each output dimension /FL
        %for iDim=1:(size(ch,1)/obj.nchans)   FIXME: conflict with
        %multimodal!
        %    [vals(iDim,:), cert(iDim,:)] = feval(obj.decf, ch(1+(iDim-1)*obj.nchans:iDim*obj.nchans,:), obj.cscale, obj.mflag, nmodes);
            [vals, cert] = feval(obj.decf, ch(1:obj.nchans,:), obj.cscale, obj.mflag, nmodes);
        %end
      % Perhaps the decoding function should only work on an integer grid.
      % We can then map from this grid to the actual input space.
      
      vals = (vals-1)*obj.ssc + obj.fpos;
      % FIXME: Different for modular?
    end
  end


  %  Decodes each pixel of a channel-coded image, created by encodeImg.
  %  Returns possibly more than one decoded image
  function varargout = decodeImg(cf, chimg, nmodes)
    if nargin<3, nmodes = 1; end;

    sz = size(chimg);
    chimg = reshape(chimg, [sz(1)*sz(2), sz(3)]);
    [img, cert] = decode(cf, chimg', nmodes);

    % Return all that is asked for (and expected)
    for ii = 1:min(nmodes, ceil(nargout/2))
      varargout{2*ii-1} = reshape(img(ii,:), sz(1:2));
      if nargout>=2*ii
        varargout{2*ii} = reshape(cert(ii,:), sz(1:2));
      end
    end

  end
  
  

  % --- Misc functions ---------------------------------------------------
  
  function M = basisMatrix(obj, nsamps)
    %
    %  M = basisMatrix(obj, nsamps)
    %
    %  Returns a matrix M, where each column is a sampled basis function.
    %  A sampled pdf 'p' can then be channel encoded by ch = M'*p. This can
    %  also be used for easy visualization, MEM reconstruction etc.
    %

    intoff = diff(obj.bounds)/2/nsamps; % first and last value should be 
    % inside, not on the boundary
    xvals = linspace(obj.bounds(1)+intoff, obj.bounds(2)-intoff, nsamps);
    M = encode(obj, xvals)';
  end
  
  
  function cc = centers(obj)
    cc = obj.fpos + [0:(obj.nchans-1)]*obj.ssc;
  end

  
end
  
end  
  
  