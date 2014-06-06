/* 
 * Code to accompany the paper:
 *   Efficient Online Structured Output Learning for Keypoint-Based Object Tracking
 *   Sam Hare, Amir Saffari, Philip H. S. Torr
 *   Computer Vision and Pattern Recognition (CVPR), 2012
 * 
 * Copyright (C) 2012 Sam Hare, Oxford Brookes University, Oxford, UK
 * 
 * This file is part of learnmatch.
 * 
 * learnmatch is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * learnmatch is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with learnmatch.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

inline unsigned int BitCount32(unsigned int v)
{
	// Fastest method for 32-bit bit-count, taken from:
	// http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
	v = v - ((v >> 1) & 0x55555555);						// reuse input as temporary
	v = (v & 0x33333333) + ((v >> 2) & 0x33333333);			// temp
	return ((v + (v >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;	// count
}

template <int Length>
inline unsigned int DotProduct(const BinaryDescriptor<Length>& d1, const BinaryDescriptor<Length>& d2)
{
    unsigned int total = 0;
	const int n = Length/32;
	const unsigned int* p1 = (const unsigned int*)d1.GetDataPtr();
	const unsigned int* p2 = (const unsigned int*)d2.GetDataPtr();
	if (n == 8)
	{
		// unroll 256-bit case
		total += BitCount32(p1[0] & p2[0]);
		total += BitCount32(p1[1] & p2[1]);
		total += BitCount32(p1[2] & p2[2]);
		total += BitCount32(p1[3] & p2[3]);
		total += BitCount32(p1[4] & p2[4]);
		total += BitCount32(p1[5] & p2[5]);
		total += BitCount32(p1[6] & p2[6]);
		total += BitCount32(p1[7] & p2[7]);
	}
	else
	{
		for (int i = 0; i < n; ++i)
		{
			total += BitCount32(p1[i] & p2[i]);
		}
	}
    return total;
}

template <int Length>
inline unsigned int HammingDistance(const BinaryDescriptor<Length>& d1, const BinaryDescriptor<Length>& d2)
{
    unsigned int total = 0;
	const int n = Length/32;
	const unsigned int* p1 = (const unsigned int*)d1.GetDataPtr();
	const unsigned int* p2 = (const unsigned int*)d2.GetDataPtr();
	if (n == 8)
	{
		// unroll 256-bit case
		total += BitCount32(p1[0] ^ p2[0]);
		total += BitCount32(p1[1] ^ p2[1]);
		total += BitCount32(p1[2] ^ p2[2]);
		total += BitCount32(p1[3] ^ p2[3]);
		total += BitCount32(p1[4] ^ p2[4]);
		total += BitCount32(p1[5] ^ p2[5]);
		total += BitCount32(p1[6] ^ p2[6]);
		total += BitCount32(p1[7] ^ p2[7]);
	}
	else
	{
		for (int i = 0; i < n; ++i)
		{
			total += BitCount32(p1[i] ^ p2[i]);
		}
	}
    return total;
}
