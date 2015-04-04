function bb = pts2bb( pts )

    if isempty(pts)
        bb = [];
        return
    end

    bb = [pts(:,1:2) pts(:,3:4)-pts(:,1:2)+1];
end