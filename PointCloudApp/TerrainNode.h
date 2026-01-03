#ifndef TERRAIN_NODE_H
#define TERRAIN_NODE_H
#include "RenderNode.h"
#include "Primitives.h"

namespace KI
{
class TerrainNode : public RenderNode
{
public:
	TerrainNode();
	~TerrainNode();

	class ComputeLODShader : public IComputeShader
	{
	public:
		ComputeLODShader() 
			: m_uInstanceCount(0)
			, m_upatchPositionNum(0) {};
		~ComputeLODShader() {};

		virtual ShaderPath GetShaderPath();
		virtual void FetchUniformLocation();
		virtual Vector3i GetLocalThreadNum() { return Vector3i(1, 1, 1); }
		void BindInstanceCount(int num);
		void BindPatchPositionNum(int num);
		void BindCamera(const GLBuffer* pBuffer);
		void BindInstance(const GLBuffer* pBuffer);
		void BindVisible(const GLBuffer* pBuffer);
		void BindDrawElementIndirect(const GLBuffer* pBuffer);


	private:
		GLuint m_uInstanceCount;
		GLuint m_upatchPositionNum;
	};


	class Shader : public IShadingShader
	{
	public:
		Shader();
		~Shader() {};

		virtual ShaderPath GetShaderPath();
		void FetchUniformLocation();
		void SetPosition(GLBuffer* pPosition);
		void SetTexcoord(GLBuffer* pTexcoord);
		void SetCamera(const GLBuffer* pBuffer);
		void SetMatrix(const GLBuffer* pBuffer);
		void BindLOD(bool value);
		void BindShowWire(bool wire);
		void BindTessLevel(int inner, int outer);
		void BindMatrix(const Matrix4x4& matrix, float scale);
		void BindHeight(float height);
		void BindTexture(const Texture& rgb, const Texture& height);
	private:
		GLuint m_uModel;
		GLuint m_uModelScale;
		GLuint m_ulevelOuter;
		GLuint m_ulevelInner;
		GLuint m_utexture;
		GLuint m_uheightMap;
		GLuint m_uHeight;
		GLuint m_ushowWire;
		GLuint m_uLOD;
	};

	virtual void Draw(const DrawContext& context);
	virtual void ShowUI(UIContext& ui);
	void BuildResource();

	struct UI
	{
		UI()
			: visible(true)
			, height(5)
			, visibleWire(false)
			, tessInner(4)
			, tessOuter(4)
			, lod(false)
		{
		}
		bool visible;
		int height;
		bool visibleWire;
		int tessInner;
		int tessOuter;
		bool lod;
	};

	struct Patch
	{
		Vector<Vector3> position;
		Vector<Vector3> texcoord;
	};

	struct PatchData
	{
		Matrix4x4 matrix;
		Vector4 center;
		float scale;
		float radius;
		uint lod;
		float padding[1];
	};

private:
	UI m_ui;
	int m_instanceCount;
	Patch m_patch;
	Unique<TerrainNode::ComputeLODShader> m_pComputeLODShader;
	Unique<TerrainNode::Shader> m_pShader;
	Unique<GLBuffer> m_pInstanceBuffer;
	Unique<GLBuffer> m_pTexcoordBuffer;
	Unique<GLBuffer> m_pPositionBuffer;

	// LOD
	Unique<GLBuffer> m_pVisibleIdBuffer;
	Unique<GLBuffer> m_pDrawIndirectBuffer;

	Unique<Texture> m_pRGB;
	Unique<Texture> m_pHeight;


};

}

#endif TERRAIN_NODE_H
