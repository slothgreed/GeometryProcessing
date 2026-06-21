#ifndef GAUSSIAN_SPLATTING_NODE
#define GAUSSIAN_SPLATTING_NODE
#include "RenderNode.h"
namespace KI
{

class GaussianSplattingData
{
public:
	GaussianSplattingData(size_t vertexNum) { m_vertexs.resize(vertexNum); };
	~GaussianSplattingData() {};

	struct Vertex
	{
		Vector4 position;
		Vector4 normal;
		Vector4 shCoeff0;
		Vector4 shCoeff1;
		Vector4 shCoeff2;
		Vector4 shCoeff3;
		Vector4 shCoeff4;
		Vector4 shCoeff5;
		Vector4 shCoeff6;
		Vector4 shCoeff7;
		Vector4 shCoeff8;
		Vector4 shCoeff9;
		Vector4 shCoeff10;
		Vector4 shCoeff11;
		Vector4 shCoeff12;
		Vector4 shCoeff13;
		Vector4 shCoeff14;
		Vector4 shCoeff15;
		Vector4 rotate;
		Vector3 scale;
		float opacity;
	};

	void SetVertex(int i, Vertex&& p) { m_vertexs[i] = std::move(p); }
	const Vector<Vertex>& GetVertexs() const { return m_vertexs; }
private:
	Vector<Vertex> m_vertexs;
};
class GaussianSplattingLoader
{
public:
	static GaussianSplattingData* Load(const String& filePath);
};



class GaussianSplattingNode : public RenderNode
{
public:
	GaussianSplattingNode(const String& name, const Shared<GaussianSplattingData>& data);
	~GaussianSplattingNode() {};
	virtual void Draw(const DrawContext& context);
	virtual void ShowUI(UIContext& ui);

	class Shader : public IComputeShader
	{
	public:
		Shader() {};
		~Shader() {};
		void SetUseAccum(bool accum) { useAccum = accum; }
		void SetDepthPhase(bool depth) { depthPhase = depth; }
		virtual Vector3i GetLocalThreadNum() const { return Vector3i(256, 1, 1); }
		virtual ShaderPath GetShaderPath();
		virtual void FetchUniformLocation();
		virtual void Execute(const DrawContext& context, const GaussianSplattingNode& node, int positionBuffer, float scale);
	private:
		bool IsScreenDispatch() { return useAccum && !depthPhase; }
		bool useAccum = false;
		bool depthPhase = false;
		GLuint m_uImageSize = 0;
		GLuint m_uVP = 0;
		GLuint m_uVertexNum = 0;
		GLuint m_uScale = 0;
	};
	const Shared<GaussianSplattingData>& GetData() const { return m_data; }
private:
	struct UI
	{
		float m_scale = 1.0f;
		bool useAccum = false;
	};
	UI m_ui;
	void BuildGLResource();
	Shared<GaussianSplattingData> m_data;
	Shared<GLBuffer> m_pVertexGpu;
	Shared<Shader> m_pShader;
	Shared<Shader> m_pWirteDepthShader;
	Shared<Shader> m_pAccumShader;
	Shared<Shader> m_pAccumeWriteDepthShader;

};


}

#endif GAUSSIAN_SPLATTING_NODE
