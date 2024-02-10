#ifndef ALPHA_SHAPE_H
#define ALPHA_SHAPE_H
#include "IAlgorithm.h"
class PointCloudNode;
class AlphaShape2D : public IAlgorithm
{
public:
	AlphaShape2D(PointCloudNode* pointCloud);
	~AlphaShape2D();

	virtual ALGORITHM_TYPE GetType() { return ALGORITHM_ALPHASHAPE; }
	virtual void Execute();
	virtual void ShowUI();
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
		Edge(const vec3& b, const vec3& e)
			: begin(b)
			, end(e)
		{
		}
		vec3 begin;
		vec3 end;
	};

	UI m_ui;
	Vector<Edge> m_edges;
	PointCloudNode* m_pPointCloud;
	float m_alpha;
};

#endif ALPHA_SHAPE_H