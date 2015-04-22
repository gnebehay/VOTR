function y = cos2kernel(x)
%
%  y = cos2kernel(x)
%
%  Evaluates the traditional cos2 kernel on the values in x,
%  using default channel width.

y = [x < 3/2 & x > -3/2] .* cos(pi/3*x).^2;

