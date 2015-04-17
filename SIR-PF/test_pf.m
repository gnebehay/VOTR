%profile on

sequence = scan_directory('juice');
track('pf', sequence, []);

%p = profile('info');
%profsave(p,'profile_results')