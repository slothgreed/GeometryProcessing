#ifndef ALPHA_SHAPE_H
#define ALPHA_SHAPE_H
#include "IAlgorithm.h"
namespace KI
{
class PointCloudNode;
class AlphaShape2D : public IAlgorithm
{
public:
	AlphaShape2D(PointCloudNode* pointCloud);
	~AlphaShape2D();

	virtual ALGORITHM_TYPE GetType() { return ALGORITHM_ALPHASHAPE; }
	virtual void Execute();
	virtual void ShowUI(RenderNode* pNode, UIContext& ui);
private:

	struct UI
	{
		UI()
			:alpha(1.0f)
		{
		}
		float alpha;
	};


	struct Edge
	{
		Edge(const Vector3& b, const Vector3& e)
			: begin(b)
			, end(e)
		{
		}
		Vector3 begin;
		Vector3 end;
	};

	UI m_ui;
	Vector<Edge> m_edges;
	PointCloudNode* m_pPointCloud;
	float m_alpha;
};
}

#endif ALPHA_SHAPE_H