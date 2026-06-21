#ifndef CROSS_SECTION_LINE_H
#define CROSS_SECTION_LINE_H
#include "IAlgorithm.h"
#include "IShader.h"
#include "RenderNode.h"
namespace KI
{
class HalfEdgeNode;
class CrossSectionLine
{
public:
	CrossSectionLine();
	~CrossSectionLine() {};
	void Draw(HalfEdgeNode* pNode, const Vector4& plane, const DrawContext& context);
	void ShowUI(HalfEdgeNode* pNode, UIContext& ui);
private:

	void Initialize(int lineNum);
	struct Shader : IComputeShader
	{
		struct Args
		{
			Vector4 plane;
			int triNum = 0;
			GLBuffer* posBuffer;
			GLBuffer* triBuffer;
			GLBuffer* lineBuffer;
			GLBuffer* drawBuffer;
		};


		Shader() {};
		virtual ~Shader() {};
		virtual Vector3i GetLocalThreadNum() const { return Vector3i(1024, 1, 1); }
		virtual void FetchUniformLocation();
		virtual ShaderPath GetShaderPath();

		void Execute(const Args& args);
		GLuint m_plane = -1;
		GLuint m_triNum = -1;
	};
	Unique<GLBuffer> m_drawBuffer;
	Unique<GLBuffer> m_lineBuffer;
	Shader m_shader;

};

}

#endif CROSS_SECTION_LINE_H