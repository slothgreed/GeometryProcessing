#ifndef MESHLET_GENERATOR_H
#define	MESHLET_GENERATOR_H
#include "HalfEdgeStruct.h"
#include "IShader.h"
namespace KI
{


struct Meshlet
{
	Meshlet()
		:maxVertex(1)
	{
	}
	typedef glm::ivec4 Cluster;
	size_t maxVertex;
	std::vector<Vector4> color;
	std::vector<glm::ivec4> data; // vec4(offset, size, 0, 0);
	std::vector<int> index;
};

class MeshletShader : public IMeshShader
{
public:
	MeshletShader() {};
	~MeshletShader() {};
	enum UNIFORM
	{
		MODEL,
		NUM
	};

	virtual ShaderPath GetShaderPath();
	void GetUniformLocation();
	void SetCamera(const GLBuffer* pBuffer);
	void SetModel(const Matrix4x4& value);
	void SetPosition(GLBuffer* pBuffer);
	void SetIndex(GLBuffer* pBuffer);
	void SetMeshlet(GLBuffer* pBuffer);
	void Draw(int first, int count);
private:
	GLuint m_uniform[UNIFORM::NUM];
};



class MeshletGenerator
{
public:
	MeshletGenerator();
	~MeshletGenerator();



	Meshlet Execute(const HalfEdgeStruct& halfEdge, int loopNum);
private:

	Meshlet ExecuteBinaryFetch(const HalfEdgeStruct& halfEdge, int loopNum);
};

}


#endif // MESHLET_GENERATOR_H