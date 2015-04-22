%ChannelEnc1D/decode
%
%  vals = decode(obj, ch)
%
%  Decodes the values in 'ch', using the decoding method specified 
%  in the Channel-encoder object.
%
%  For the standard basis functions, there are standard decoders
%  available, but a custom decoder can also be specified by setting the
%  field 'decf' in the object. A custom decoder should be a function looking
%  as follows:
%
%    [vals, conf] = decode_func(ch)
%      ch:    MxN matrix of channel values
%
%      vals:  RxN matrix of decoded values, assuming channels centers are
%             at 1,2,... The first row contains the strongest peaks and
%             the other rows weaker peaks. Returning more than one row is
%             optional.
%
%      conf:  RxN matrix of confidences for each decoded value. 
%
%
%  [Erik Jonsson, 2006]

