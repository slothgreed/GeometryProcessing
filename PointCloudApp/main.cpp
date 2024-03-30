
#include "PointCloudApp.h"
#include "ComputePointCloudApp.h"
#include "ComputeShaderTest.h"
int main()
{
	//ComputeShaderTest app;
	//KI::ComputePointCloudApp app;
	KI::PointCloudApp app;
	app.Initialize();
	app.Execute();
	app.Finalize();
	return 0;
}
