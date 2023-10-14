#ifndef IALGORITHM_H
#define IALGORITHM_H
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

enum ALGORITHM_TYPE
{
	ALGORITHM_KDTREE,
	ALGORITHM_HARRIS3D,
	ALGORITHM_ALPHASHAPE
};
class IAlgorithm
{
public:
	IAlgorithm() {};
	~IAlgorithm() {};
	virtual ALGORITHM_TYPE GetType() = 0;
	virtual void Execute() = 0;
	virtual void ShowUI() = 0;
private:

};

#endif