#ifndef DRAE_CALL_TEST_H
#define	DRAE_CALL_TEST_H

#include "CameraController.h"
#include "IAlgorithm.h"
class PointCloudApp
{
public:
	PointCloudApp() {};
	~PointCloudApp() {};

	void Execute();
	void Finalize();
	void ProcessMouseEvent(const MouseInput& input);

private:
	std::unordered_map<ALGORITHM_TYPE, IAlgorithm*> m_algorithm;
	std::unique_ptr<Mouse> m_pMouse;
	std::shared_ptr<PerspectiveCamera> m_pCamera;
	std::unique_ptr<CameraController> m_pCameraController;
};

static PointCloudApp* Application();

#endif // DRAE_CALL_TEST_H