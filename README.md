# Visual Object Tracking Repository

This repository aims at collecting state-of-the-art tracking algorithms that are freely available.
In addition, each tracker that is listed here was adapted as to implement a simple standard calling convention
to make automated comparison of tracker results more easy.

Currently, the following trackers are contained in this repository:

* [CppMT](https://www.github.com/gnebehay/CppMT)
* [CT](https://www.github.com/gnebehay/CT)
* [DSST](https://www.github.com/gnebehay/DSST)
* [FragTrack](https://www.github.com/gnebehay/FragTrack)
* [HoughTrack](https://www.github.com/gnebehay/HoughTrack)
* [LearnMatch](https://www.github.com/gnebehay/LearnMatch)
* [SCM](https://www.github.com/gnebehay/SCM)
* [SIR-PF](https://www.github.com/gnebehay/SIR-PF)
* [STRUCK](https://www.github.com/gnebehay/STRUCK)
* [TLD](https://www.github.com/gnebehay/TLD)
* [qwsEDFT](https://www.github.com/gnebehay/qwsEDFT)

The calling convention follows the file protocol of the [VOT challenge](http://www.votchallenge.net).
In this calling convention, the tracker can expected to find the files images.txt and region.txt
that contain the individual image file names and the initialization region respectively.
After tracking, the file output.txt is written by the tracker into the folder.
The python files in this repository can be used to run a tracker on a given sequence.
It is necessary to copy the file .cv-example to your home folder ~/.cv and adapt the contents accordingly
for these scripts to work.
A suitable dataset can be found [here](http://www.gnebehay.com/cmt/). 
