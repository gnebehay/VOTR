function samples = sample_gaussian(M, C, N)
% SAMPLE_GAUSSIAN 
% Draw N random row vectors from a Gaussian distribution
% samples = sample_gaussian(mean, covariance, N)

if nargin == 2
  N = 1;
end

% If Y = CX, Var(Y) = C Var(X) C'.
% So if Var(X)=I, and we want Var(Y)=Sigma, we need to find C. s.t. Sigma = C C'.
% Since Sigma is psd, we have Sigma = U D U' = (U D^0.5) (D'^0.5 U').

M = M(:);
n = length(M);
[U, D, ~] = svd(C);
samples = randn(n, N);
samples = (U * sqrt(D)) * samples + M * ones(1,N); % transform each column
samples = samples';
