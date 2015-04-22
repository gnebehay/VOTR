DSST: Discriminative Scale Space Tracker
-------------------------------------------------------------------------------

DSST is part of the [Visual Object Tracking Repository](https://github.com/gnebehay/VOTR),
which aims at providing a central repository for state-of-the-art tracking algorithms that are freely available.
The source code for this tracker was submitted for the [VOT2014 challenge](https://www.votchallenge.net).
A minor adaption was made by using MATLAB's imresize() function
instead of a custom mex file to increase portability.

Dependencies:
* Matlab
* Image Processing Toolbox
* Signal Processing Toolbox

The following description was copied literally from the original author.

ABSTRACT
----------------------------------------------------------------------------

The Discriminative Scale Space Tracker (DSST), proposed in [2], extends the Minimum Output Sum of Squared
Errors (MOSSE) tracker [1] with robust scale estimation. The MOSSE tracker works by training a discriminative
correlation filter on a set of observed sample grayscale patches. This correlation filter is then applied to estimate the
target translation in the next frame. The DSST additionally learns a one-dimensional discriminative scale filter, that
is used to estimate the target size. The scale filter is trained by extracting several sample patches at different scales
around the current target position in the image. Each sample is represented by a fixed-length feature vector based on
HOG. These samples are used to learn a multi-channel one-dimensional discriminative filter for scale estimation. This
scale filter is generic and can be combined with any tracker that is limited to only estimating the target translation.
Given a new image, the DSST first applies a translation filter to obtain the most probable target location. The scale
filter is then applied at this location to estimate the target size. The tracking model is then updated with the new
information information of the target and background appearance. For the translation filter, we combine the intensity
features employed in the MOSSE tracker with a pixel-dense representation of HOG-features.

[1] D. S. Bolme, J. R. Beveridge, B. A. Draper, and Y. M. Lui. Visual object tracking using adaptive correlation filters. In CVPR,
2010.

[2] M. Danelljan, G. Häger, F. Shahbaz Khan, and M. Felsberg. Accurate scale estimation for robust visual tracking. In Proceedings
of the British Machine Vision Conference (BMVC), 2014.

README
----------------------------------------------------------------------------

This MATLAB code integrates the Discriminative Scale Space Tracker (DSST) [1] into the VOT 2014 evaluation kit. The implementation is built upon the code provided by [2]. The code provided by [3] is used for computing the HOG features.

Instructions:
Use the wrapper function "wrapper.m" without trax.
The code may be publicly available from the VOT homepage.

Contact:
Martin Danelljan
martin.danelljan@liu.se


[1] Martin Danelljan, Gustav Häger, Fahad Shahbaz Khan and Michael Felsberg.
    "Accurate Scale Estimation for Robust Visual Tracking".
    Proceedings of the British Machine Vision Conference (BMVC), 2014.

[2] J. Henriques, R. Caseiro, P. Martins, and J. Batista.
    "Exploiting the circulant structure of tracking-by-detection with kernels."
    In ECCV, 2012.

[3] Piotr Dollár.
    "Piotr's Image and Video Matlab Toolbox (PMT)."
    http://vision.ucsd.edu/~pdollar/toolbox/doc/index.html.
