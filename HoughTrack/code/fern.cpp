/******************************************************************************
 * Author: Martin Godec
 *         godec@icg.tugraz.at
 ******************************************************************************/

#include "fern.h"

int Node::MapStep = MAP_STEP;
int Node::MapSize = MAP_SIZE;

using namespace cv;

bool sortVotesDesc (const std::pair<CvPoint, float>& A, const std::pair<CvPoint, float>& B)
{
	return (A.second > B.second);
}

bool sortFernsDesc (const Fern& A, const Fern& B)
{
	return A.getTableSize() > B.getTableSize();
}

Fern::Fern( const Size& baseSize, unsigned int numTests, unsigned int numChannels )
: m_baseSize(baseSize), m_numTests(numTests), numPos(1), numNeg(1)
{
	for(unsigned int t = 0; t < numTests; t++)
		m_tests.push_back( RandomTest(baseSize, numChannels) );

	m_nodeTable.clear();
}

Fern::~Fern()
{
	m_tests.clear();
	m_nodeTable.clear();
}

void Fern::evaluate(Features& ft, const Rect& ROI, Mat& result, int stepSize, float threshold) const
{
	std::map< unsigned int, Node >::const_iterator it;
	std::map< unsigned int, Node >::const_iterator end = m_nodeTable.end();

	for(int x = ROI.x; x < (ROI.x + ROI.width - m_baseSize.width); x+=stepSize)
	{
		for(int y = ROI.y; y < (ROI.y + ROI.height - m_baseSize.height); y+=stepSize)
		{
			it = m_nodeTable.find( calcIndex(ft, Point(x,y)) );

			if(it != end)
			{
				const Node& node = (*it).second;

				if(node.probPos > threshold)
				{

					std::vector< std::pair< Point, float > > votes = node.getVotes();
					
					for(unsigned int v = 0; v < votes.size(); v++)
					{
						Point pos = Point(x + votes.at(v).first.x, y + votes.at(v).first.y);

						if((pos.x >= 0) && (pos.y >= 0) && (pos.x < result.cols) && (pos.y < result.rows))
							result.at<float>( pos.y, pos.x ) += votes.at(v).second;
					}
				}

			}
		}
	}
}

int Fern::backProject(Features& ft, Mat& projected, const Rect& ROI, Point& center, float radius, int stepSize, float threshold) const
{
	std::map< unsigned int, Node >::const_iterator it;
	std::map< unsigned int, Node >::const_iterator end = m_nodeTable.end();
	float max_dist_sq = radius * radius;

	int cnt = 0;

	for(int x = ROI.x; x < (ROI.x + ROI.width); x+=stepSize)
	{
		for(int y = ROI.y; y < (ROI.y + ROI.height); y+=stepSize)
		{
			unsigned char prior = projected.at<unsigned char>( y, x );
			if(prior == GC_FGD)
				continue;
			
			it = m_nodeTable.find( calcIndex(ft, Point(x, y)) );

			if(it != end)
			{
				const Node& node = (*it).second;

				if(node.probPos > threshold)
				{
					std::vector< std::pair< Point, float > > votes = node.getVotes();

					//projected.at<unsigned char>( y, x ) = GC_PR_FGD;

					for(unsigned int v = 0; v < votes.size(); v++)
					{
						Point pos = Point(x + votes.at(v).first.x, y + votes.at(v).first.y);
						float dist_sq = static_cast<float>(pow(pos.x-center.x, 2.0f) + pow(pos.y-center.y, 2.0f));

						if(dist_sq <= max_dist_sq)
						{
							cnt++;
							projected.at<unsigned char>( y, x ) = GC_FGD;
						}
					}
				}
			}
		}
	}

	return cnt;
}

void Fern::forget(const double& factor)
{
	std::map< unsigned int, Node >::iterator it = m_nodeTable.begin();
	while(it != m_nodeTable.end())
	{
		(*it++).second.forget(factor);
	}
	
}

void Fern::clear()
{
	std::map< unsigned int, Node >::iterator it = m_nodeTable.begin();
	while(it != m_nodeTable.end())
	{
		(*it++).second.clear();
	}
	numPos = 1.0f;
	numNeg = 1.0f;
}

void Fern::update(Features& ft, const Point& pos, int label, const Point& center)
{
	unsigned int idx = calcIndex(ft, pos);
	std::map< unsigned int, Node >::iterator it = m_nodeTable.find( idx );

	if(it == m_nodeTable.end())
	{
		// insert new node
		Node newnode;
		it = m_nodeTable.insert ( std::make_pair( idx, newnode) ).first;
	}

	Node& node = (*it).second;
	Point vote = Point(center.x-pos.x, center.y-pos.y);
	
	// update node
	if((label == GC_FGD) || (label == GC_PR_FGD))
	{
		node.updateMap( vote );
		node.numPos += 1.0f;
		numPos += 1.0f;
	}
	else if(label == GC_BGD)
	{
		node.numNeg += 1.0f;
		numNeg += 1.0f;
	}
	else
	{
		std::cerr << "unknown label " << label << std::endl;
		return;
	}

	//float posRatio = node.numPos / numPos;
	//float negRatio = node.numNeg / numNeg;
	float negPosRatio = numNeg / numPos;
	node.probPos = negPosRatio * node.numPos / (negPosRatio * node.numPos + node.numNeg);//posRatio / (negRatio + posRatio);
	
//	node.probPos = static_cast<float>(node.numPos) / static_cast<float>(node.numPos + node.numNeg); // how to adjust number of samples?
}

