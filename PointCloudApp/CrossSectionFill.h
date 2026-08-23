#ifndef CROSS_SECTION_FILL_H
#define CROSS_SECTION_FILL_H
#include "IAlgorithm.h"
#include "IShader.h"
#include "RenderNode.h"
namespace KI
{
class HalfEdgeNode;
class CrossSectionFill
{
public:
	CrossSectionFill();
	~CrossSectionFill() {};
	void Draw(HalfEdgeNode* pNode, const Vector4& plane, const DrawContext& context);
	void ShowUI(HalfEdgeNode* pNode, UIContext& ui);
private:
	struct Shader : IShadingShader
	{
		virtual ShaderPath GetShaderPath() override;
		virtual void FetchUniformLocation() override;
		virtual void SetModel(const Matrix4x4& value) override;
		void SetCamera(const GLBuffer* pBuffer);
		void SetPosition(const GLBuffer* pBuffer);
		void SetColor(const Vector3& value);
		void SetPlane(const Vector4& value);
		void SetHatch(bool enabled, const Vector3& color, float spacing, float lineWidth);

		GLuint m_model = -1;
		GLuint m_color = -1;
		GLuint m_plane = -1;
		GLuint m_hatchEnabled = -1;
		GLuint m_hatchColor = -1;
		GLuint m_hatchSpacing = -1;
		GLuint m_hatchLineWidth = -1;
	};

	void UpdatePlane(const HalfEdgeNode* pNode, const Vector4& plane);
	void DrawMesh(HalfEdgeNode* pNode, const Vector4& plane, const Vector3& color, const DrawContext& context);

	Shader m_shader;
	Unique<GLBuffer> m_planePosition;
	Unique<GLBuffer> m_planeIndex;
	bool m_hatchEnabled = true;
	int m_hatchLineCount = 24;
	float m_hatchLineWidth = 0.08f;
};

}

#endif CROSS_SECTION_FILL_H
