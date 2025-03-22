#ifndef SHAPE_DIAMETER_FUNCTION
#define SHAPE_DIAMETER_FUNCTION
#include "IAlgorithm.h"
#include "KIMath.h"
namespace KI
{
class HalfEdgeNode;
class ShapeDiameterFunction : public IAlgorithm
{
public:
	ShapeDiameterFunction(HalfEdgeNode* pNode);
	~ShapeDiameterFunction();

	virtual ALGORITHM_TYPE GetType() override { return ALGORITHM_SHAPE_DIAMETER; }
	virtual void Execute();
	virtual void ShowUI(UIContext& ui);

	const Vector<float>& GetResult() const { return m_result; }
	Vector<Vector3> GetResultFaceColor() const;
	Vector<Vector3> GetResultVertexColor() const;
private:

	int m_debugIndex;
	float m_maxValue;
	HalfEdgeNode* m_pHalfEdge;
	Vector<float> m_result;
	int m_samplingNum;

};

}

#endif SHAPE_DIAMETER_FUNCTION
