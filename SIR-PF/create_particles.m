function X = create_particles(M, C, N, WH)
	XY = sample_gaussian(M, C, N)'; %x y position
	V = zeros(2, N); %x' y' velocity

	X = [XY; V; repmat(WH,1,N)];