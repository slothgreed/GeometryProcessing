#ifndef COMPUTE_SHADER_TEST_H
#define	COMPUTE_SHADER_TEST_H
#include "TheApp.h"
namespace KI
{
class ComputeShaderTest : public TheApp
{
public:
	ComputeShaderTest();
	~ComputeShaderTest();
	void Initialize();
	void Execute();
	void Finalize();
private:

};
}
#endif // COMPUTE_SHADER_TEST_H