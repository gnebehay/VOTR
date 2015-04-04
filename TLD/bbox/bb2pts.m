function pts=bb2Ppts(bb)

    if isempty(bb)
        pts = [];
        return;
    end

    pts = [bbTL(bb) bbBR(bb)];
end