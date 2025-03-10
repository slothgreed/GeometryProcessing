#ifndef GLTF_LOADER_H
#define GLTF_LOADER_H
#include "GLTFStruct.h"
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
namespace KI
{
class BDB;
class GLTFDocument
{
public:
	GLTFDocument();
	~GLTFDocument();
	const Microsoft::glTF::Document& GetDocument() const;
	void SetDocument(Microsoft::glTF::Document&& document);
private:
	class Impl;
	GLTFDocument::Impl* m_pImpl;
};
class Texture;
class GLTFLoader
{
public:
	GLTFLoader() {};
	~GLTFLoader() {};

	static RenderNode* Load(const String& name);
private:

	static Vector<GLTFSkin> LoadSkin(const Microsoft::glTF::Document* pDocument, const Microsoft::glTF::GLTFResourceReader* pResource);
	static Vector<GLTFAnimation> LoadAnimation(const Microsoft::glTF::GLTFResourceReader* pResource, const Microsoft::glTF::Document* pDocument);
	static Vector<GLTFMaterial> LoadMaterial(const Microsoft::glTF::Document* pDocument, const Vector<Shared<Texture>>& textures);
	static Vector<GLTFMesh> LoadMesh(const Microsoft::glTF::GLTFResourceReader* pResource, const Microsoft::glTF::Document* pDocument, MeshBuffer& pBuffer, BDB& bdb);
	static Vector<GLTFNode> LoadNode(const Microsoft::glTF::Document* pDocument);
	static Vector<Shared<Texture>> LoadTexture(const String& directory, const Microsoft::glTF::Document* pDocument);
};
}


#endif GLTF_LOADER_H

