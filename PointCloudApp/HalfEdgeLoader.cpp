#include "HalfEdgeLoader.h"
#include "FileUtility.h"
namespace KI
{
HalfEdgeStruct* HalfEdgeLoader::Load(const String& filePath)
{
	KI::FileReader reader;
	reader.Open(filePath, true);

	int version = reader.ReadInt();
	int vertexSize = reader.ReadInt();
	int edgeSize = reader.ReadInt();
	int faceSize = reader.ReadInt();
	
	if (version != 2 ||
		vertexSize == 0 ||
		edgeSize == 0 ||
		faceSize == 0) {
		assert(0);
	}

	Vector<HalfEdge> halfEdge(edgeSize);
	Vector<Vector3> position(vertexSize);
	Vector<int> positionToEdge(edgeSize);
	Vector<int> faceToEdge(faceSize);
	struct IndexedVertex
	{
		float x;
		float y;
		float z;
		int index;
	};

	auto vertexData = reader.ReadStruct<IndexedVertex>(vertexSize);
	for (int i = 0; i < vertexSize; i++) {
		position[i] = Vector3(vertexData[i].z, vertexData[i].y, vertexData[i].x);
		positionToEdge[i] = vertexData[i].index;
	}

	halfEdge = reader.ReadStruct<HalfEdge>(edgeSize);
	faceToEdge = reader.ReadInt(faceSize);
	reader.Close();

	HalfEdgeStruct* data = new HalfEdgeStruct();
	data->Set(
		std::move(position),
		std::move(halfEdge),
		std::move(positionToEdge),
		std::move(faceToEdge));

	return data;
}
}


