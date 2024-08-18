#ifndef KI_GLTF_SCENE_H
#define KI_GLTF_SCENE_H
#include "GLTFStruct.h"
namespace KI
{
class GLTFShader;
class GLTFChannelUpdaterOnGpu;
class GLTFSceneMatrixUpdaterOnGpu;
class GLTFSkinUpdaterOnGpu;
class GLTFScene : public RenderNode
{
public:
	GLTFScene(const String& name)
		: RenderNode(name)
		, m_debugView(0)
		, m_pShader(nullptr)
		, m_pChannelGpuUpdater(nullptr)
		, m_pMatrixGpuUpdater(nullptr)
		, m_pSkinGpuUpdater(nullptr)
		, m_visible(true)
	{
	};
	virtual ~GLTFScene();

	void Initialize();
	void SetNode(Vector<GLTFNode>&& node) { m_nodes = std::move(node); };
	void SetRoot(Vector<int>&& root) { m_roots = std::move(root); }
	void SetMaterial(Vector<GLTFMaterial>&& value) { m_material = std::move(value); }
	void SetTexture(Vector<Shared<Texture>>&& value) { m_texture = std::move(value); }
	void SetMeshBuffer(Vector<MeshBuffer>&& buffer) { m_meshBuffer = std::move(buffer); }
	void SetSkin(Vector<GLTFSkin>&& value) { m_skins = std::move(value); }
	void SetMesh(Vector<GLTFMesh>&& value) { m_meshes = std::move(value); }
	void SetAnimation(Vector<GLTFAnimation>&& animation) { m_animation = std::move(animation); }
	virtual void ShowUI();
	virtual void DrawNode(const DrawContext& context);
protected:
	virtual void UpdateData(float time);
private:
	void InitMatrix();
	void UpdateMatrix();
	
	void InitAnimation();
	void UpdateAnimation(float time);
	
	void InitSkeleton();
	void UpdateSkin();

	void CreateMaterialBuffer();
	GLTFShader* m_pShader;
	Vector<MeshBuffer> m_meshBuffer;

	GLTFSkinUpdaterOnGpu* m_pSkinGpuUpdater;
	GLTFChannelUpdaterOnGpu* m_pChannelGpuUpdater;
	GLTFSceneMatrixUpdaterOnGpu* m_pMatrixGpuUpdater;

	Vector<int> m_roots;
	Vector<Shared<Texture>> m_texture;
	Vector<GLTFMaterial> m_material;
	Vector<GLTFNode> m_nodes;
	Vector<GLTFSkin> m_skins;
	Vector<GLTFMesh> m_meshes;
	Vector<GLTFAnimation> m_animation;
	bool m_visible;
	struct GpuObject
	{
		GpuObject()
			: materialBuffer(nullptr)
			, nodeBuffer(nullptr)
			, skinBuffer(nullptr)
		{
		}
		GLBuffer* materialBuffer;

		GLBuffer* nodeBuffer;
		Vector<GLTFNode::GpuObject> node;

		Vector<GLBuffer*> channelBuffer;
		Vector<Vector<GLTFAnimation::ChannelGpuObject>> channel;
		
		Vector<GLBuffer*> samplerBuffer;
		Vector<Vector<GLTFAnimation::SamplerGpuObject>> sampler;

		GLBuffer* skinBuffer;
	};
	int m_debugView;
	GpuObject m_gpu;

};

}
#endif KI_GLTF_SCENE_H