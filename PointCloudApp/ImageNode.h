#ifndef IMAGE_NODE_H
#define IMAGE_NODE_H
#include "RenderNode.h"
#include "IShader.h"
#include "Texture.h"
#include "Primitive.h"
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