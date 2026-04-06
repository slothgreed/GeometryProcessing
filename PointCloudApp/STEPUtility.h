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
}
}
#endif KI_STEP_UTILITY_H