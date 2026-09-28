#ifndef HALF_EDGE_LOADER_H
#define HALF_EDGE_LOADER_H
#include "Algorithm/HalfEdge/HalfEdgeStruct.h"
namespace KI
{
class HalfEdgeLoader
{
public:
	HalfEdgeLoader() {};
	~HalfEdgeLoader() {};

	
	static HalfEdgeStruct* Load(const String& filePath);
private:

};

}


#endif HALF_EDGE_LOADER_H
