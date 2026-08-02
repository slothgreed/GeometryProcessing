#ifndef MODEL_CLASSIFICATION_H
#define	MODEL_CLASSIFICATION_H
#include "GLFWApp.h"
#include "DebugNode.h"
#include <filesystem>
#include <vector>
#include <string>
namespace KI
{
class ModelClassification : public GLFWApp
{
public:
	ModelClassification();
	~ModelClassification();
	void Initialize();
	void Execute();
	void Finalize();

	virtual void ProcessMouseEvent(const MouseInput& input);
	virtual void ResizeEvent(int width, int height);

private:

	struct UI
	{
		int m_selectClassIndex = 0;
		int m_selectFileIndex = 0;
	};

	UI m_ui;
	struct ModelNetData
	{
		std::string className;
		std::vector<std::filesystem::path> files;
	};

	Vector<ModelNetData> LoadModelNetTrain(const std::filesystem::path& root);
	void LoadData(const String& offFile);


	Vector<ModelNetData> m_data;
	void ShowUI();
	Shared<RenderResource> m_pResource;
	Shared<MeshNode> m_pMeshNode;
};
}

#endif // MODEL_CLASSIFICATION_H