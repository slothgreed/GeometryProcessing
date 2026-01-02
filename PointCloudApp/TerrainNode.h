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
		{
		}
		bool visible;
		int height;
		bool visibleWire;
		int tessInner;
		int tessOuter;

	};

	struct Patch
	{
		Vector<Vector3> position;
		Vector<Vector3> texcoord;
	};

	struct InstanceData
	{
		Matrix4x4 matrix;
		float scale;
		float padding[3];
	};

private:
	UI m_ui;
	Patch m_patch;
	Unique<TerrainNode::Shader> m_pShader;
	Unique<GLBuffer> m_pInstanceBuffer;
	Unique<GLBuffer> m_pTexcoordBuffer;
	Unique<GLBuffer> m_pPositionBuffer;
	Unique<Texture> m_pRGB;
	Unique<Texture> m_pHeight;


};

}

#endif TERRAIN_NODE_H
