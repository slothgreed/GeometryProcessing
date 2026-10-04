#ifndef IMAGE_NODE_H
#define IMAGE_NODE_H
#include "GL/Buffer/Texture.h"
#include "Node/RenderNode.h"
#include "Primitive/Primitive.h"
#include "Renderer/Shader/IShader.h"
namespace KI
{
class ImageNode : public RenderNode
{
public:
	ImageNode(const String& name, const Shared<Texture>& pTexture);
	~ImageNode() {};

	void DrawNode(const DrawContext& context);
protected:
	virtual void ShowUI(UIContext& ui);
private:
	struct UI
	{
		UI() 
			: visible(true)
			, outline(false)
			, outlineDebugCount(-1)
		{}
		bool visible;
		bool outline;
		int outlineDebugCount;
	};

	UI m_ui;
	void BuildGLBuffer();
	Shared<Texture> m_pTexture;
};
}

#endif IMAGE_NODE_H
