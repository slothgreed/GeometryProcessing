#ifndef KI_STEP_UTILITY_H
#define KI_STEP_UTILITY_H
#include "STEPTypes.h"
#include "Polyline.h"
namespace KI
{
	namespace STEP
	{
		class Intersection
		{
		public:
			Intersection() {};
			~Intersection() {};

			static Polyline CreatePolyline(STEPInterSectionCurve* pIntersectionCurve);
		private:
		};

	}
}
#endif KI_STEP_UTILITY_H