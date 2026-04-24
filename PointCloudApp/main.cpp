#include "PointCloudApp.h"
#include "ComputePointCloudApp.h"
#include "ComputeShaderTest.h"
#include "MeshShaderTest.h"
#include "SoftwareRasterizer.h"
#include <random>
#include "Profiler.h"
#include "SIMDAPI.h"

void SIMDTest()
{
	const int ARRAY_SIZE = 80000000;
	std::vector<float> a(ARRAY_SIZE);
	std::vector<float> b(ARRAY_SIZE);
	for (size_t i = 0; i < a.size(); i++) {
		a[i] = i;
		b[i] = i;
	}
	KI::CPUProfiler profiler;
	int mode = 0;
	if (mode == 0) {

		std::vector<float> c(ARRAY_SIZE, 0);
		profiler.Start();
		for (int i = 0; i < a.size(); i++) {
			c[i] = a[i] + b[i];
		}
		profiler.Stop();
		profiler.Output("Array Add Normal");

		profiler.Start();
		for (size_t i = 0; i < a.size(); i += 8) {
			auto va = KI::SIMD::F32X8(&a[i]);
			auto vb = KI::SIMD::F32X8(&b[i]);
			auto vc = KI::SIMD::F32X8::Add(va, vb);
			vc.Store(&c[i]);
		}
		profiler.Stop();
		profiler.Output("Array Add SIMD");
	} else if (mode == 1) {
		int sum = 0;
		profiler.Start();	
		for (int i = 0; i < a.size(); i++) {
			sum += a[i];
		}
		profiler.Stop();
		profiler.Output("Array Sum" + KI::IntToString(sum));


		sum = 0;
		profiler.Start();
		for (int i = 0; i < a.size(); i+=16) {
			sum += a[i];
		}
		profiler.Stop();
		profiler.Output("Array Sum 16" + KI::IntToString(sum));
	}



}
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