DSST: Discriminative Scale Space Tracker (DSST)
-------------------------------------------------------------------------------

DSST is part of the [Visual Object Tracking Repository](https://github.com/gnebehay/VOTR),
which aims at providing a central repository for state-of-the-art tracking algorithms that are freely available.
The source code for this tracker was submitted for the [VOT2014 challenge](https://www.votchallenge.net).
A minor adaption was made by using MATLAB's imresize() function
instead of a custom mex file to increase portability.
The following description was copied literally from the original author.

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
