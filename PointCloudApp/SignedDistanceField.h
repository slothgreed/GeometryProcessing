#ifndef SIGNED_DISTANCE_FIELD
#define SIGNED_DISTANCE_FIELD
#include "IAlgorithm.h"
#include "RenderResource.h"
#include "BVH.h"
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
	virtual void ShowUI(UIContext& ui);
private:

	enum Axis
	{
		X,Y,Z
	};


	BVH::IntersectResult CalcMinDistance(const Vector3& pos) const;
	void CreateTexure(int resolute);
	void CreateSDFTexture(int resolute, float position, Axis axis, std::vector<Vector3>& lines);

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
		int resolute;
		Plane xPlane;
		Plane yPlane;
		Plane zPlane;
	};

	struct Gpu
	{
		Shared<Texture2D> xTexture;
		Shared<Texture2D> yTexture;
		Shared<Texture2D> zTexture;
	};

	Gpu m_gpu;
	UI m_ui;
	HalfEdgeNode* m_pHalfEdge;
	float m_resolute;
};

}

#endif SIGNED_DISTANCE_FIELD
