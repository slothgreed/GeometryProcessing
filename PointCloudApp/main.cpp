#include "PointCloudApp.h"
#include "ComputePointCloudApp.h"
#include "ComputeShaderTest.h"
#include "MeshShaderTest.h"
#include "SoftwareRasterizer.h"
#include <random>

int main()
{
	std::cout << std::fixed << std::setprecision(2); // •‚“®¬”“_2Œ…‚Ü‚Å
    //KI::ComputeShaderTest app;
	//KI::ComputePointCloudApp app;
	//KI::PointCloudApp app;
	KI::SoftwareRasterizer app;
	//KI::MeshShaderTest app;
	app.Initialize();
	app.Execute();
	app.Finalize();

	return 0;
}