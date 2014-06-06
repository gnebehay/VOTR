CT: Compressive Tracking
-------------------------------------------------------------------------------

Compressive Tracking is part of the [Visual Object Tracking Repository](https://github.com/gnebehay/VOTR),
which aims at providing a central repository for state-of-the-art tracking algorithms that are freely available.
The source code for this tracker was obtained from its [project website](http://www4.comp.polyu.edu.hk/~cslzhang/CT/CT.htm)
and extended by a challenge mode.
The following description was copied literally from the original author.

README
----------------------------------------------------------------------------------------------------------------------------------------------
* C++ demo for "Real-Time Compressive Tracking," Kaihua Zhang, Lei Zhang, Ming-Hsuan Yang, ECCV 2012.
* Author: Yang Xian
* Email: yang_xian521@163.com
* Revised by Kaihua Zhang
* Project website:  http://www4.comp.polyu.edu.hk/~cslzhang/CT/CT.htm
* Revised date: 23/8/2012
* Version 1
-----------------------------------------------------------------------------------------------------------------------------------------------
This code requires both OpenCV 2.4.2 (http://sourceforge.net/projects/opencvlibrary/files/opencv-win/) to be installed on your machine.  It has only been tested on a machine running Windows XP, usiong Visual Studio 2008.  In order for the code to run, make sure you have the OpenCV bin directory in your system path.
Use at own risk.  Please send us your feedback/suggestions/bugs.
----------------------------------------------------------------------------------------------------------------------------------------------
> put the image sequence in 'compressivTracking/data'

> set the initial rectangle in 'compressivTracking/config.txt'

> run the code
----------------------------------------------------------------------------------------------------------------------------------------------
Tracking results will be saved in file "CompressiveTracking/TrackingResults.txt". Each line in the file contains the [x y width height].
----------------------------------------------------------------------------------------------------------------------------------------------
Note: the results shown by our paper is based on our MATLAB code. The results by this c++ code may be somewhat different from the results by our MATLAB code because there exist randomness in the code.

Thank you! Enjoy it!
