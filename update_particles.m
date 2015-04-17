function X = update_particles(X, model, w_pos, w_vec, w_size)
	N = size(X, 2);
	
	%Predict move
	X = model * X;
	
	%Add noise to prediction
	X(1:2,:) = X(1:2,:) + w_pos * randn(2, N);
	X(3:4,:) = X(3:4,:) + w_vec * randn(2, N);
	X(5:6,:) = max(1, X(5:6,:) + w_size * randn(2, N));