#ifndef IMAGE_ALGORITHM_H
#define IMAGE_ALGORITHM_H
#include "Texture.h"
#include "Polyline.h"
namespace KI
{
class ImageAlgorithm
{
public:
	ImageAlgorithm() {};
	~ImageAlgorithm() {};



	static Polyline CreateOutlineByMooreNeighbor(const PixelData& data, int loopNum = -1);
private:

};

}

#endif IMAGE_ALGORITHM_H
