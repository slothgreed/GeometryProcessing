#include "STEPUtility.h"
namespace KI
{
namespace STEP
{

Polyline Intersection::CreatePolyline(const STEPInterSectionCurve::Data& data, const Vector3& begin, const Vector3& end)
{
	if (data.type == STEPInterSectionCurve::Data::Type::Curve3D) {
		if (data.curve0.pCircle) {
			return data.curve0.pCircle->CreatePolyline(begin, end);
		} else if(data.curve0.pLine){
			Polyline line;
			line.Add(begin); line.Add(end);
			return line;
		}

	} else {
		assert(0);
	}

	return Polyline();
}

}
}