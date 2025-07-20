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

	struct Cluster
	{
		int offset;
		int size;
		int meshletIndex;
		int padding;
		Vector4 boxMin;
		Vector4 boxMax;
		Vector4 normal;
	};

	size_t maxVertex;
	std::vector<Cluster> cluster; // vec4(offset, size, meshletIndex, 0);
	std::vector<int> index;
};

class MeshletShader : public IMeshShader
{
public:
	MeshletShader() {};
	~MeshletShader() {};

	virtual int GetTaskThreadNum() const { return 32; }
	virtual int GetMeshThreadNum() const { return 32; }

	virtual ShaderPath GetShaderPath();
	void FetchUniformLocation();
	void SetCamera(const GLBuffer* pBuffer);
	void SetModel(const Matrix4x4& value);
	void SetNormalMatrix(const Matrix3x3& value);
	void SetMeshletNum(int num);
	void SetTaskToMeshNum(const GLBuffer* pBuffer);
	void SetPosition(GLBuffer* pBuffer);
	void SetIndex(GLBuffer* pBuffer);
	void SetMeshlet(GLBuffer* pBuffer);
	void SetCullSize(int cullSize);
private:
	GLuint u_CullSize;
	GLuint u_Model;
	GLuint u_NormalMatrix;
	GLuint u_MeshletNum;
};



class MeshletGenerator
{
public:
	MeshletGenerator();
	~MeshletGenerator();



	static Meshlet Execute(const HalfEdgeStruct& halfEdge, int loopNum);
private:
	static Meshlet ExecuteBinaryFetch(const HalfEdgeStruct& halfEdge, int loopNum);
};


class MeshletProfiler
{
public:
    MeshletProfiler();
    ~MeshletProfiler();

    void BeginQuery();
    void EndQuery();
    void ShowUI(); // ImGui‚ÅŒ‹‰Ê‚ð•\Ž¦

private:
    GLuint m_primitiveQuery = 0;
    GLuint m_vertexQuery = 0;

    GLuint64 m_primitives = 0;
    GLuint64 m_vertices = 0;
};

}


#endif // MESHLET_GENERATOR_H