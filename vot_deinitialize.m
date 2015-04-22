function vot_deinitialize(results)

if size(results, 2) ~= 8
	error('Illegal result format');
end;



csvwrite('output.txt', results);



