HoughTrack: Hough-based Tracking
-------------------------------------------------------------------------------

HoughTrack is part of the [Visual Object Tracking Repository](https://github.com/gnebehay/VOTR),
which aims at providing a central repository for state-of-the-art tracking algorithms that are freely available.
The source code for this tracker was obtained from its [project website](http://lrs.icg.tugraz.at/research/houghtrack/)
and extended by a challenge mode.
The following description was copied literally from the original author.

-------------------------------------------------------------------------------
  README
-------------------------------------------------------------------------------

This is the original implementation of the "Hough-based Tracking of Non-Rigid
Objects" Paper by Martin Godec (godec@icg.tugraz,at). This source-code is free
for personal and academic use. If you use it for a scientific publication,
please cite

  @INPROCEEDINGS{godec11HoughTracking,
    author = {Martin Godec and Peter M. Roth and Horst Bischof},
    title = {Hough-based Tracking of Non-rigid Objects},
    booktitle = {Proc. Int. Conf. on Computer Vision},
    year = {2011}
  }

For for information or commercial use, please have a look at the LICENCE file.

-------------------------------------------------------------------------------
  REQUIREMENTS
-------------------------------------------------------------------------------

We use several different libraries for reading/writing images, configuration,
folder parsing,...

CMake 2.6+: used to create the Makefile; install from www.cmake.org or package
  manager (Linux)

OpenCV 2.1: used for image reading/writing, image manipulation, grab-cut
  segmentation,...; install from opencv.willowgarage.com, should work with newer
  versions with minimal changes in the "CMakeLists.txt" (TARGET_LINK_LIBRARIES)

Boost 1.40: used for folder parsing in capture.cpp/h; install from www.boost.org
  or package manager (Linux). Installer for Windows www.boostpro.com. Should
  also work with newer versions of boost.

Config++: used for configuration of capture and main file. Please have a look at
  the "sample/sample.conf" file; install from www.hyperrealm.com/libconfig/

-------------------------------------------------------------------------------
  TESTED ENVIRONMENT
-------------------------------------------------------------------------------

Hardware
  Intel Core2 Duo E8500 @ 3.16GHz
  4GB RAM

OS, Software and Libraries
  Ubuntu 10.04 (lucid)
  CMake 2.8 (installed via Package Manager)
  GCC 4.4.3 (installed via Package Manager 4:4.4.3-1ubuntu1)
  Boost 1.40 (installed via Package Manager)
  OpenCV 2.1 (manually compiled and installed)
  Config++ 1.4.7 (manually compiled and installed)

-------------------------------------------------------------------------------
  HOW TO START
-------------------------------------------------------------------------------

- Install the required packages/software

- Type the following commands to compile the software

  cmake .
  make

- Start the tracker using the provided sample
  ./Track sample/sample.conf

- Copy/Edit the provided demo configuration to apply the tracker to your own
  sequences. If the parameter "startRegion" is given, the tracker will start
  using the object at this position. If you comment/delete this parameter, you
  can mark the object yourself. The application will provide you the very first
  frame of the sequence. Press the left button on your mouse and mark the object
  in the image. You can redraw the rectangle until you hit the right button of
  your mouse, this will start the tracking process.

-------------------------------------------------------------------------------
  COMMENTS
-------------------------------------------------------------------------------

In this implementation, we use the feature extraction from the original Hough-
Forest paper for better comparability. The author is stated in the files.

