STRUCK: Structured Output Tracking with Kernels
-------------------------------------------------------------------------------

STRUCK is part of the [Visual Object Tracking Repository](https://github.com/gnebehay/VOTR),
which aims at providing a central repository for state-of-the-art tracking algorithms that are freely available.
The source code for this tracker was obtained from its [project website](http://www.samhare.net/research/struck/code)
and extended by a challenge mode.
Additionally, the code was updated to use Eigen3 and to support OpenCV 3.
The following description was copied literally from the original author.

README
-------------------------------------------------------------------------------

Code to accompany the paper:
  Struck: Structured Output Tracking with Kernels
  Sam Hare, Amir Saffari, Philip H. S. Torr
  International Conference on Computer Vision (ICCV), 2011

Copyright (C) 2011 Sam Hare, Oxford Brookes University, Oxford, UK

Contact: Sam Hare <sam.hare@brookes.ac.uk>

------------
Requirements
------------

OpenCV: http://opencv.willowgarage.com/
Eigen: http://eigen.tuxfamily.org/

This code has been developed and tested using 
OpenCV v2.1 and Eigen v2.0.15

-----
Usage
-----

> struck [config-file-path]

If no path is given the application will attempt to
use ./config.txt.

Please see config.txt for configuration options.

---------
Sequences
---------

Sequences are assumed to be of the format of those 
available from:

http://vision.ucsd.edu/~bbabenko/project_miltrack.shtml

----------------
Acknowledgements
----------------

This code uses the OpenCV graphing utilities provided
by Shervin Emami: http://www.shervinemami.co.cc/graphs.html
