
#include "PointCloudApp.h"
#include "ComputePointCloudApp.h"
#include "ComputeShaderTest.h"
#include "MeshShaderTest.h"
int main()
{
	//KI::ComputeShaderTest app;
	//KI::ComputePointCloudApp app;
	KI::PointCloudApp app;
	//KI::MeshShaderTest app;
	app.Initialize();
	app.Execute();
	app.Finalize();

	return 0;
}
