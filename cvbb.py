import numpy as np

#   OVERLAP = CALCULATE_OVERLAP(T1, T2) calculates overlap between trajectories
#       T1 and T2, where T1 and T2 are matrices of size N1 x 4 and N2 x 4, where
#       the corresponding columns for each matrix describe the upper left and top
#       coordinate as well as width and height of the bounding box.

def overlap(T1,T2):

    #Check for equal length
    if not T1.shape[0] == T2.shape[0]:
        raise Exception('Number of entries is inconsistent')

    hrzInt = np.minimum(T1[:, 0] + T1[:, 2], T2[:, 0] + T2[:, 2]) - np.maximum(T1[:, 0], T2[:, 0])
    hrzInt = np.maximum(0,hrzInt)
    vrtInt = np.minimum(T1[:, 1] + T1[:, 3], T2[:, 1] + T2[:, 3]) - np.maximum(T1[:, 1], T2[:, 1])
    vrtInt = np.maximum(0,vrtInt)
    intersection = hrzInt * vrtInt

    union = (T1[:, 2] * T1[:, 3]) + (T2[:, 2] * T2[:, 3]) - intersection

    overlap = intersection / union

    return overlap

def br(bbs):

    result = np.hstack((bbs[:,[0]] + bbs[:,[2]]-1, bbs[:,[1]] + bbs[:,[3]]-1))

    return result

def tl(bbs):

    result = bbs[:,:2]

    return result

def pts2bb(pts):

    bbs = np.hstack((pts[:,:2], pts[:,2:4]-pts[:,:2]+1))

    return bbs

def bb2pts(bbs):

    pts = np.hstack((bbs[:,:2], br(bbs)))

    return pts

def bb2poly(bbs):

    return pts2poly(bb2pts(bbs))

def pts2poly(pts):

    x_min = pts[:,[0]]
    y_min = pts[:,[1]]
    x_max = pts[:,[2]]
    y_max = pts[:,[3]]

    poly = np.hstack((x_min,y_min,x_max,y_min,x_max,y_max,x_min,y_max))

    return poly

def poly2bb(poly):
    x_coords = poly[:,::2]
    y_coords = poly[:,1::2]

    min_x = np.min(x_coords, axis=1)
    min_y = np.min(y_coords, axis=1)
    max_x = np.max(x_coords, axis=1)
    max_y = np.max(y_coords, axis=1)

    A = np.vstack((min_x, min_y, max_x, max_y)).T

    A = pts2bb(A)

    return A

def write(fname, bbs):
    np.savetxt(fname, bbs, fmt='%.2f', delimiter=',')

def read(fname):
    bbs = np.genfromtxt(fname, delimiter=',')
    return bbs
