/******************************************************************************
 * Author: Martin Godec
 *         godec@icg.tugraz.at
 ******************************************************************************/

#include "utilities.h"

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cmath>
#include <sstream>
#include <iostream>
#include <iomanip>

#ifdef WIN32
#define random() rand()
#endif

using namespace std;

double randDouble() {
    static bool didSeeding = false;

    if (!didSeeding) {
        srand(time(0));
        didSeeding = true;
    }
    return random()/(RAND_MAX + 1.0);
}

double randnDouble() {
	static bool didSeeding = false;

    if (!didSeeding) {
        srand(time(0));
        didSeeding = true;
    }
    return 2.0*(random()/(RAND_MAX + 1.0)) - 1.0;
}

int randIntFromRange(const int from, const int range) {
    return from + int(range*randDouble());
}

// adapted from GNU Scientific Library
double randGauss( double std_dev )
{
    static bool didSeeding = false;

    if (!didSeeding) {
        srand(0);
        didSeeding = true;
    }


  double x, y, r2;

  do
    {
      /* choose x,y in uniform square (-1,-1) to (+1,+1) */
      x = -1.0 + 2.0 * (random() / (double)RAND_MAX);
      y = -1.0 + 2.0 * (random() / (double)RAND_MAX);

      /* see if it is in the unit circle */
      r2 = x * x + y * y;
    }
  while (r2 > 1.0 || r2 == 0);

  /* Box-Muller transform */
  return std_dev * y * sqrt (-2.0 * log (r2) / r2);
}

std::string createFilename(std::string prefix, int idx, std::string suffix, int numLength)
{
	std::stringstream filename;
	filename << prefix << std::setfill ('0') << std::setw (numLength) << idx << suffix;
	return filename.str();
}

void setCenter(Rect& rect, const Point& center)
{
	rect.x = center.x - rect.width/2;
	rect.y = center.y - rect.height/2;
}

Point getCenter(const Rect& rect)
{
	return Point(rect.x + rect.width/2, rect.y + rect.height/2);
}

Rect intersect(const Rect& rectA, const Rect& rectB)
{
	int intersectionX = ( rectA.x > rectB.x ) ? rectA.x : rectB.x;
	int intersectionY = ( rectA.y > rectB.y ) ? rectA.y : rectB.y;
	int intersectionWidth = ( ( rectA.x + rectA.width ) < ( rectB.x + rectB.width ) ) ? ( rectA.x + rectA.width - intersectionX ) : ( rectB.x + rectB.width - intersectionX );
	int intersectionHeight = ( ( rectA.y + rectA.height ) < ( rectB.y + rectB.height ) ) ? ( rectA.y + rectA.height - intersectionY ) : ( rectB.y + rectB.height - intersectionY );

	if ( intersectionHeight < 0 || intersectionWidth < 0 )
		return Rect(0,0,0,0);

	return Rect ( intersectionX, intersectionY, intersectionWidth, intersectionHeight );
}


