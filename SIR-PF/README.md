SIR-PF: Sequential Importance Re-sampling Particle Filter
-------------------------------------------------------------------------------

DSST is part of the [Visual Object Tracking Repository](https://github.com/gnebehay/VOTR),
which aims at providing a central repository for state-of-the-art tracking algorithms that are freely available.
The source code for this tracker was submitted for the [VOT2014 challenge](https://www.votchallenge.net).
The following description was copied literally from the original author.

README
----------------------------------------------------------------------------

For this challenge I decided to make an extended implementation of SIR Particle Filter tracker. Main aspects of the
implementation where to make basic Particle Filter more robust on sequences with fast motion, illumination changes
and more faster in terms of execution time. Therefore in this implementation I focused on the idea of changing normal
visual RGB model to Y Cb Cr, which was the most promising, while testing on various sequences with illumination
changes. Furthermore, because this visual model was not enough to make tracker more stable, I also decided to include
window adaptation (only on logical level), reference histogram adaptation (only if the matching with newly calculated
histogram is 90%-95% and also some sort of background removal method. First method for background removal was
used from D. Comaniciu etal. [1], that gave good results with examples of tracking single objects or objects different
from other background ones. Therefore my final implementation contains a cheap trick, of using reference window size
smaller for 30% from original selected region. This method with comparison of basic background removal method,
gives more stable and better results for major sequences proposed with VOT2013 and also in this VOT2014 challenge.
But still in the end, the tracker is not that robust on collisions between similar objects, small but fatal image distortions
(blurriness, uneven sharpening, uneven illumination changes, etc.) and window shape/rotation changes.

[1] D. Comaniciu, V. Ramesh, and P. Meer. Kernel-Based Object Tracking. IEEE Transactions on Pattern Analysis and Machine
Intelligence, 25:564–575, 2003.

[2] The VOT 2014 evaluation kit. http://www.votchallenge.net.
