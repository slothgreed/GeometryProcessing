#ifndef GLTF_LOADER_H
#define GLTF_LOADER_H
class RenderNode;
namespace Microsoft
{
	namespace glTF
	{
		struct Node;
		struct BufferView;
		class Document;
		class GLTFResourceReader;
	}
}
class GLTFDocument
{
public:
	GLTFDocument();
	~GLTFDocument();
	const Microsoft::glTF::Document& GetDocument();
	void SetDocument(Microsoft::glTF::Document&& document);
private:
	class Impl;
	GLTFDocument::Impl* m_pImpl;
};

class GLTFLoader
{
public:
	GLTFLoader() {};
	~GLTFLoader() {};

	static RenderNode* Load(const String& name);
private:

};

#endif GLTF_LOADER_H

