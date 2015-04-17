function [result] = track(tracker, sequence, region, varargin)
% TRACK performs a tracking run on a given sequence with a specified
% tracker. The tracker should be a string <tracker> such that the functions
% tracker_<tracker>_initialize and tracker_<tracker>_update are available
% in the path.
%
% Additional arguments:
% - interactive: show GUI and prompt for initialization if not given
% - initialize: cell array of additional arguments passed to initialization
%               function
% - trajectory: display the past position of object on the image
% - pause: interval of pause between frames (if 0 or less you have to press
%               a key to proceed)

interactive = true;
pause_interval = 0.1;
initialize = {};
show_trajectory = false;

for i = 1:2:length(varargin)
    switch lower(varargin{i})
        case 'interactive'
            interactive = varargin{i+1}; 
        case 'initialize'
            initialize = varargin{i+1}; 
        case 'trajectory'
            show_trajectory = varargin{i+1};
        case 'pause'
            pause_interval = varargin{i+1}; 
        otherwise 
            error(['Unknown switch ', varargin{i},'!']) ;
    end
end 

tracker_initialize = str2func(['tracker_', tracker, '_initialize']);
tracker_update = str2func(['tracker_', tracker, '_update']);

I = imread(sequence{1});

if isempty(region) && interactive
    hf = figure();
    imshow(I);
    region = getrect(hf);
    close(hf);
end;

result = zeros(length(sequence), 4);

[state, location] = tracker_initialize(I, region, initialize{:});

result(1, :) = location;

for i = 2:length(sequence)

    I = imread(sequence{i});
	
    if interactive
        figure(1);
        imshow(I);
        hold on;
		
		[state, location] = tracker_update(state, I);

		if isempty(location)
			location = nan(1, 4);
		end;

		result(i, :) = location;
		
        rectangle = [location(1:2)', location(1:2)' + location(3:4)'];
        plot(rectangle(1, [1, 1, 2, 2, 1]), rectangle(2, [1, 2, 2, 1, 1]), 'r');
        
        if show_trajectory
            plot(result(1:i, 1) + result(1:i, 3) / 2, ...
                result(1:i, 2) + result(1:i, 4) / 2, 'r', 'LineWidth', 2);
        end;
        
        hold off;
        if pause_interval <= 0
            pause();
        else
            pause(pause_interval);
        end;
    end;
    
end;
