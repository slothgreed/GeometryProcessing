#ifndef POINT_CLOUD_NODE_H
#define POINT_CLOUD_NODE_H
#include "RenderNode.h"
#include "SimpleShader.h"
#include "IAlgorithm.h"
#include "DelaunayGenerator.h"
namespace KI
{
class PointCloud;
class PointCloudNode : public RenderNode
{
public:
	PointCloudNode(const String& name, Shared<PointCloud>& pPrimitive);
	~PointCloudNode();

	const Shared<PointCloud>& GetData() const;
	void UpdateData();
	void DrawNode(const DrawContext& context);
	virtual void ShowUI(UIContext& ui);
	const Vector<Vector3>& GetNormal();
	const Vector<int>& GetNeighbor(int index);

private:

	class Shader : public IComputeShader
	{
	public:
		Shader():depthPhase(false) {};
		~Shader() {};
		void SetDepthPhase(bool depth) { depthPhase = depth; }
		virtual ShaderPath GetShaderPath();
		virtual void FetchUniformLocation();
		virtual void Execute(const DrawContext& context, const PointCloudNode& node, int positionBuffer);
	private:
		bool depthPhase;
		GLuint m_uImageSize;
		GLuint m_uVP;
		GLuint m_uPositionNum;
	};
	
	void UpdateColor(const Vector<Vector4>& color);
	void ComputeNormal();
	void ComputeNeighbor(float radius);
	void ShowNormal();
	void ComputeTangent();
	void BuildGLBuffer();
	void UpdateRenderData();
	DelaunayGenerator m_delaunay;
	Vector<Vector3> m_normal;
	Vector<Vector3> m_tangentX;
	Vector<Vector3> m_tangentY;
	std::unordered_map<ALGORITHM_TYPE, IAlgorithm*> m_algorithm;
	Shared<PointCloud> m_pPointCloud;
	Vector<Vector<int>> m_neighbor;
	Unique<GLBuffer> m_pInterleave;
	Unique<GLBuffer> m_pPositionBuffer;
	Vector3 m_color;
	Unique<GLBuffer> m_pColorBuffer;
	Shader m_shader;
	Shader m_shaderWriteDepth;
};

}

#endif POINT_CLOUD_NODE_H