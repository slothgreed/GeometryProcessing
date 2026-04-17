#ifndef KI_STEP_UTILITY_H
#define KI_STEP_UTILITY_H
#include "STEPTypes.h"
#include "Polyline.h"
#include "STEPEntity.h"

namespace KI
{
namespace STEP
{
class Intersection
{
public:
	Intersection() {};
	~Intersection() {};

	static Polyline CreatePolyline(const STEPInterSectionCurve::Data& pIntersectionCurve, const Vector3& begin, const Vector3& end);
private:

};

class BSplineBuilder
{
public:
	static Polyline CreatePolyline(const STEPBSplineCurve& bspline, int sampleCount, const Vector3& begin, const Vector3& end);
	static Mesh CreateMesh(const STEPBSplineSurface& bspline, int uSegments, int vSegments);
private:
	// Curve
	static Vector<float> CreateExpandedKnots(const Vector<int>& multiple, const Vector<float>& knots);
	static bool CheckKnotSize(const Vector<float>& knots, int pointNum, int degree);

	static float Basis(int i, int p, float u, const Vector<float>& expandKnots);
	static bool GetParameterRange(const Vector<float>& expandKnots, int degree, float& outMin, float& outMax);
	static bool Evaluate(const STEPBSplineCurve& bspline, float u, Vector3& outPoint);
	static Polyline CreatePolyline_(const STEPBSplineCurve& bspline, int sampleCount, const Vector3& begin, const Vector3& end);

	// Surface
	static bool Evaluate(const STEPBSplineSurface& bspline, const STEPUV<Vector<float>>& expandKnots, float u, float v, Vector3& outPoint);



};
}
}
#endif KI_STEP_UTILITY_H