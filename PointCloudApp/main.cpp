#include "PointCloudApp.h"
#include "ComputePointCloudApp.h"
#include "ComputeShaderTest.h"
#include "MeshShaderTest.h"
#include "SoftwareRasterizer.h"
#include <random>
#include "Profiler.h"
#include "ProcessExecutor.h"
#include "MeshViewer.h"
#include "AIDataGenerator.h"
#include "HalfEdgeStruct.h"
#include "HalfEdgeLoader.h"
int main()
{
	
	//KI::AIDataGenerator generator;
	//auto data = generator.LoadMeshsegBenchmark("E:\\cgModel\\MeshsegBenchmark-1.0\\data");
	//generator.SaveMeshCNN(data);
	//std::string path = "E:\\cgModel\\bunny6000.half";
	//auto pBunny = std::shared_ptr<KI::HalfEdgeStruct>(KI::HalfEdgeLoader::Load(path));
	//KI::String outPath = "E:\\cgModel\\MeshsegBenchmark-1.0\\data\\predict\\bunny.meshcnn";
	//generator.SaveMeshCNN(outPath, *pBunny);
	//return 0;

	std::cout << std::fixed << std::setprecision(2); // •‚“®¬”“_2Œ…‚Ü‚Å
    
	KI::AIProcessor::Instance().ExecuteASync("--named");
	KI::RadixSortTest app;
	//KI::PrefixSumTest app;
	//KI::HistogramTest app;
	//KI::ComputeShaderTest app;
	//KI::ComputePointCloudApp app;
	//KI::PointCloudApp app;
	//KI::SoftwareRasterizer app;
	//KI::MeshShaderTest app;
	//KI::MeshViewer app;
	app.Initialize();
	app.Execute();
	app.Finalize();


	KI::AIProcessor::Instance().FinalizeASync();
	return 0;
}