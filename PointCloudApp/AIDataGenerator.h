#ifndef AI_DATA_GENERATOR_H
#define AI_DATA_GENERATOR_H
#include <filesystem>
namespace KI
{
class Mesh;
class HalfEdgeStruct;
struct AIDataFolder
{
	struct Data
	{
		std::filesystem::path filePath;
		std::vector<std::filesystem::path> segFile;
	};
	std::string className;
	std::vector<Data> datas;
};
class AIDataGenerator
{
public:
	AIDataGenerator();
	~AIDataGenerator();

	static Vector<AIDataFolder> LoadModelNetTrain(const std::filesystem::path& root);
	static Vector<AIDataFolder> LoadMeshsegBenchmark(const std::filesystem::path& root);
	static Vector<Vector3> LoadMeshsegBenchmarkSeg(const std::filesystem::path& path);
	static Vector<Vector3> LoadSegBinary(const std::string& path);

	// ModelNet40のtrainフォルダを指定して、各offファイルをサンプリングしてpoint cloudに変換する。
	static void SavePointCloud(const String& folderName, int sampleCount);
	// MeshCNN用のデータセットを作成する。
	static void SaveMeshCNN(const Vector<AIDataFolder>& folder);
	static void SaveMeshCNN(const String& fileName, const HalfEdgeStruct& halfEdge);
private:
	static void ConvertTrainFolder(const std::filesystem::path& trainFolder, int sampleCount);
	static void ConvertFile(const std::filesystem::path& sourceFile, const std::filesystem::path& destinationFile, int sampleCount);

};

}

#endif AI_DATA_GENERATOR_H
