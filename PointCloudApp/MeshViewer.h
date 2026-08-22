#ifndef MODEL_CLASSIFICATION_H
#define	MODEL_CLASSIFICATION_H
#include "GLFWApp.h"
#include "DebugNode.h"
#include "AIDataGenerator.h"
#include <vector>
#include <string>
namespace KI
{
class PrimitiveNode;


class MeshViewer : public GLFWApp
{
public:
	MeshViewer();
	~MeshViewer();
	void Initialize();
	void Execute();
	void Finalize();

	virtual void ProcessMouseEvent(const MouseInput& input);
	virtual void ResizeEvent(int width, int height);

private:
	enum LoadType
	{
		MODEL_CLASSIFICATION,
		MESH_SEGMENTATION,
		LOAD_BUNNY,
		DIFFUSION_AI
	};
	LoadType m_loadType = MODEL_CLASSIFICATION;

	struct UI
	{
		bool animation = false;
		bool m_garallyMode = false;
		int m_selectCategoryIndex = 0;
		int m_selectFileIndex = 0;
		int m_selectSegIndex = 0;
	};

	UI m_ui;
	

	void UpdateMeshCategory();
	struct AIDataFolderInfo
	{
		int maxFileNum = 0;
		int maxSegNum = 0;
	};


	struct GaralleyData
	{
		int maxPageNum = 40;
		int width = 8;
		int height = 5;
		int selectPage = 0;
		int segIndex = 0;
	};

	GaralleyData m_garalleyData;
	Vector<Shared<RenderNode>> m_pGaralleyNode;
	void UpdateGaralley();
	void DrawGaralley(const DrawContext& context);
	void ShowGaralleyUI();
	void UpdateRenderData();
	int GetGaralleyPageNum() const;
	
	Shared<RenderNode> LoadModelNetData(const AIDataFolder& folder, int fileIndex);
	Shared<RenderNode> LoadMeshsegData(const AIDataFolder& folder, int fileIndex, int segIndex);
	Shared<RenderNode> LoadMeshCNNBunny();
	Shared<RenderNode> LoadMeshPointCloudBynny();

	float m_timerDiff = 0.0f;
	Vector<AIDataFolder> m_folder;
	AIDataFolderInfo m_folderInfo;
	void ShowUI();
	bool ShowSliderUI(const String& name, int minValue, int maxValue, int* ret);
	Shared<RenderResource> m_pResource;
	Shared<RenderNode> m_pRenderNode;

};
}

#endif // MODEL_CLASSIFICATION_H
