FragTrack: Fragments-based Tracking
-------------------------------------------------------------------------------

FragTrack is part of the [Visual Object Tracking Repository](https://github.com/gnebehay/VOTR),
which aims at providing a central repository for state-of-the-art tracking algorithms that are freely available.
The source code for this tracker was obtained from its [project website](http://www.cs.technion.ac.il/~amita/fragtrack/fragtrack.htm)
and extended by a challenge mode.
The following description was copied literally from the original author.

README
-----------------------------------------

By: 	Amit Adam

	amita@cs.technion.ac.il
	www.cs.technion.ac.il/~amita

Date:	November 18th, 2007

-----------------------------------------


General
-------

This distribution contains the source code for a fragments-based tracker.
It is written in C++ and uses the OpenCV library.


What's in the package
---------------------

1. Fragments_Tracker.h,cpp - the tracker object code
2. fragtrack_envelope.cpp - an envelope for running the tracker on an image sequence
3. emd.h,cpp - code for comparing two histograms using Earth Mover's Distance - 
   courtesy of Yossi Rubner 
4. A Visual Studio solution for building the project.
5. Sample setup files for two image sequence. The sequences may be found in my
   homepage.
6. Sample log file.


Usage
-----

1. Build the executable - a console application (tested only in "release configuration")
2. Prepare a setup file called "setup.txt" and place it in the same directory
   as the executable file.
3. Run the executable.
4. The following output should be obtained:

	- during the run - an OpenCV window with the tracking results
	- a log file called "FragTrack_log.txt" containing the tracking results
	- two images "initial_temlate.jpg" and "initial_target.jpg" showing the initial template


Format of setup file
--------------------

The setup file is a text file containing 7 lines in the following format:



F:\\amita\\data\\face_sequence\\        	% line 1 - path and file name prefix
1						% line 2 - first image in sequence	
890						% line 3 - last image in sequence
75 120 220 235 					% line 4 - target position in first image
7						% line 5 - search window half size
16						% line 6 - number of bins in histogram
3						% line 7 - choice of metric for comparing histograms


(do not include the comments in the setup file)

Here are some details:


The first 3 lines specify where to find the input sequence - in the above example the sequence is
F:\amita\data\face_sequence\ 1.jpg, F:\amita\data\face_sequence\2.jpg, ..., F:\amita\data\face_sequence\890.jpg
Note: no spaces are allowed in path or file name

Line number 4 gives the top-left and bottom-right corners of the target position in the first frame.
The y-coordinate (row) is given first: tl_y tl_x br_y br_x

Line number 5 specifies the search radius around the position in previous frame (in pixels).

The algorithm is based on gray-scale intensity histograms. Line number 6 specifies the number of bins
in the histograms.

For comparing two histograms the algorithm currently uses one of three options. Line 7 specifies which option:
1 means chi-square metric, 2 means EMD metric, 3 means a variation of the Kolmogorov-Smirnov statistic. The EMD
is a cross-bin metric in contrast with standard bin-to-bin metrics such as Chi square. 
For one dimensional data option 3 is a much faster equivalent to the EMD metric. Option 3 should be
your default choice. You can see the advantage option 3 has over option 1 on the "woman" sequence for example.


Example setup files and sequences
---------------------------------

Two example setup files are contained in the distribution. The corresponding sequences are available from

	www.cs.technion.ac.il/~amita

together with a file containing the ground truth for these sequences.


Feedback
--------

Feedback (both positive and negative) is most welcome. Please email to amita@cs.technion.ac.il


Acknowledgement
---------------

Thanks to Yossi Rubner for his EMD code and for permission to redistribute it with this package.


Reference
---------

Amit Adam, Ehud Rivlin, Ilan Shimshoni: Robust Fragments-based Tracking using the Integral Histogram.
Proc. CVPR 2006, pp. 798-805

 
