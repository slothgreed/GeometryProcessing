#include "PointCloudApp.h"
#include "ComputePointCloudApp.h"
#include "ComputeShaderTest.h"
#include "MeshShaderTest.h"
#include "SoftwareRasterizer.h"
#include <random>
#include "Profiler.h"
#include "ClientPipe.h"
#include "ProcessExecutor.h"


void sendClose()
{

}

int main()
{
	std::cout << std::fixed << std::setprecision(2); // •‚“®¬”“_2Œ…‚Ü‚Å
    
	KI::ProcessExecutor executor("GeometryAI.exe");
	executor.ExecuteASync("--named");
	KI::ClientPipe clientPipe;
	clientPipe.Open();
    clientPipe.SendCommand("C:\\test\\sample.stl");
    clientPipe.Close();
    executor.FinalizeASync();
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




	return 0;
}