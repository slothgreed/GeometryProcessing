#ifndef SIGNED_DISTANCE_FIELD
#define SIGNED_DISTANCE_FIELD
#include "IAlgorithm.h"
#include "RenderResource.h"
namespace KI
{
class HalfEdgeNode;
class SignedDistanceField : public IAlgorithm
{
public:
	SignedDistanceField(HalfEdgeNode* pNode);
	~SignedDistanceField();

	virtual ALGORITHM_TYPE GetType() override { return ALGORITHM_SIGNED_DISTANCE_FIELD; }
	virtual void Execute();
	virtual void ShowUI();
private:
	struct UI
	{
		struct Plane
		{
			Plane() 
				: visible(false)
				, position(0.0f)
			{
			}
			bool visible;
			float position;
		};
		UI();
		Shared<GLStatus> glStatus;
		Plane xPlane;
		Plane yPlane;
		Plane zPlane;
	};

	UI m_ui;
	HalfEdgeNode* m_pHalfEdge;
};

}

#endif SIGNED_DISTANCE_FIELD
