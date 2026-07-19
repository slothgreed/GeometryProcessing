#include "PointCloudApp.h"
#include "ComputePointCloudApp.h"
#include "ComputeShaderTest.h"
#include "MeshShaderTest.h"
#include "SoftwareRasterizer.h"
#include <random>
#include "Profiler.h"
#include "ProcessExecutor.h"



int main()
{
	std::cout << std::fixed << std::setprecision(2); // •‚“®¬”“_2Œ…‚Ü‚Å
    
	KI::AIProcessor::Instance().ExecuteASync("--named");
	//KI::RadixSortTest app;
	//KI::PrefixSumTest app;
	//KI::HistogramTest app;
	//KI::ComputeShaderTest app;
	//KI::ComputePointCloudApp app;
	KI::PointCloudApp app;
	//KI::SoftwareRasterizer app;
	//KI::MeshShaderTest app;
	app.Initialize();
	app.Execute();
	app.Finalize();


	KI::AIProcessor::Instance().FinalizeASync();
	return 0;
}