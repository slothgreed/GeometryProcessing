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

	for (int i = 0; i < vertexSize; i++) {
		position[i] = vec3(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
		positionToEdge[i] = reader.ReadInt();
	}

	int end = 0, next = 0, before = 0, opposite = 0, face = 0;
	for (int i = 0; i < edgeSize; i++) {
		HalfEdge edge;
		edge.endPos = reader.ReadInt();
		edge.nextEdge = reader.ReadInt();
		edge.beforeEdge = reader.ReadInt();
		edge.oppositeEdge = reader.ReadInt();
		edge.face = reader.ReadInt();
		halfEdge[i] = std::move(edge);
	}

	for (int i = 0; i < faceSize; i++) {
		faceToEdge[i] = reader.ReadInt();
	}
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


