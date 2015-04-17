function [sequence] = scan_directory(dir)
% SCAN_DIRECTORY scans a directory for images matching the following
% pattern:
%   00000001.jpg, 00000002.jpg, 00000003.jpg ...
% and returns an ordered cell array of the full file paths to the
% available images that form a video sequence.

sequence = cell(0, 0);

i = 0;

mask = '%08d.jpg';

while true
    i = i + 1;
    
    image_name = sprintf(mask, i);

    if ~exist(fullfile(dir, image_name), 'file')
        break;
    end;

    sequence{i} = fullfile(dir, image_name);

end;
