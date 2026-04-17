#include "STEPUtility.h"
#include "KIMath.h"
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

bool BSplineBuilder::CheckKnotSize(const Vector<float>& knots, int pointNum, int degree)
{
	return (knots.size() == pointNum + degree + 1);
}


Vector<float> BSplineBuilder::CreateExpandedKnots(const Vector<int>& multiple, const Vector<float>& knots)
{
	Vector<float> outExpandKnots;
	if (multiple.size() != knots.size()) { return Vector<float>(); }

	for (int i = 0; i < (int)knots.size(); ++i) {
		if (multiple[i] <= 0) { return Vector<float>(); }
		for (int j = 0; j < multiple[i]; ++j) {
			outExpandKnots.push_back(knots[i]);
		}
	}

	return outExpandKnots;
}

Polyline BSplineBuilder::CreatePolyline(const STEPBSplineCurve& bspline, int sampleCount, const Vector3& begin, const Vector3& end)
{
	auto ret = CreatePolyline_(bspline, sampleCount, begin, end);
	if (ret.LineNum() == 0) {
		ret.Add(begin); ret.Add(end);
	}
	return ret;
}


float BSplineBuilder::Basis(int i, int p, float u, const Vector<float>& knots)
{
	if (p == 0) {
		if ((knots[i] <= u && u < knots[i + 1]) ||
			(u == knots.back() && i + 1 == (int)knots.size() - 1)) {
			return 1.0f;
		}
		return 0.0f;
	}

	float left = 0.0f;
	float right = 0.0f;

	const float leftDenom = knots[i + p] - knots[i];
	if (leftDenom != 0.0f) {
		left = (u - knots[i]) / leftDenom * Basis(i, p - 1, u, knots);
	}

	const float rightDenom = knots[i + p + 1] - knots[i + 1];
	if (rightDenom != 0.0f) {
		right = (knots[i + p + 1] - u) / rightDenom * Basis(i + 1, p - 1, u, knots);
	}

	return left + right;
}

bool BSplineBuilder::GetParameterRange(const Vector<float>& expandKnots, int degree, float& outMin, float& outMax)
{
	const int minIndex = degree;
	const int maxIndex = (int)expandKnots.size() - 1 - degree;

	if (minIndex < 0 || minIndex > expandKnots.size()) { return false; }
	if (maxIndex < 0 || maxIndex > expandKnots.size()) { return false; }

	outMin = expandKnots[minIndex];
	outMax = expandKnots[maxIndex];
	return true;
}
bool BSplineBuilder::Evaluate(const STEPBSplineCurve& bspline, float u, Vector3& outPoint)
{
	if ((int)bspline.expandKnots.size() != bspline.points.size() + bspline.degree + 1) { return false; }

	const bool hasWeight = !bspline.rational.empty();
	if (hasWeight && bspline.rational.size() != bspline.points.size()) { return false; }

	Vector3 numerator{ 0.0f, 0.0f, 0.0f };
	float denominator = 0.0f;

	for (int i = 0; i < bspline.points.size(); ++i) {
		const float N = Basis(i, bspline.degree, u, bspline.expandKnots);
		if (N == 0.0f) { continue; }

		const float w = hasWeight ? bspline.rational[i] : 1.0f;
		const Vector3 P = bspline.points[i].second->pos;

		numerator += P * (N * w);
		denominator += (N * w);
	}

	if (denominator == 0.0f) {
		return false;
	}

	outPoint = numerator / denominator;
	return true;
}

Polyline BSplineBuilder::CreatePolyline_(const STEPBSplineCurve& bspline, int sampleCount, const Vector3& begin, const Vector3& end)
{
	if (sampleCount < 2) { return Polyline(); }

	float uMin = 0.0f;
	float uMax = 0.0f;
	if (!GetParameterRange(bspline.expandKnots, bspline.degree, uMin, uMax)) { return Polyline(); }

	Vector<Vector3> points;
	for (int i = 0; i < sampleCount; ++i) {
		const float t = (float)i / (float)(sampleCount - 1);
		const float u = uMin + (uMax - uMin) * t;

		Vector3 p;
		if (!Evaluate(bspline, u, p)) {
			if (i == sampleCount - 1) {
				p = end;
			} else {
				continue;
			}
		}

		points.push_back(p);
	}

	return Polyline(std::move(points),Polyline::DrawType::LineStrip).ConvertLines();
}






bool BSplineBuilder::Evaluate(const STEPBSplineSurface& bspline, const STEPUV<Vector<float>>& expandKnots, float u, float v, Vector3& outPoint)
{
	Vector3 numerator{ 0.0f, 0.0f, 0.0f };
	float denominator = 0.0f;

	const bool hasWeight = !bspline.rational.empty();
	for (int i = 0; i < bspline.GetUNum(); ++i) {
		const float Nu = Basis(i, bspline.degree.u, u, expandKnots.u);
		if (Nu == 0.0f) { continue; }

		for (int j = 0; j < bspline.GetVNum(); ++j) {
			const float Nv = Basis(j, bspline.degree.v, v, expandKnots.v);
			if (Nv == 0.0f) { continue; }

			const float w = hasWeight ? bspline.rational[i][j] : 1.0f;
			const float B = Nu * Nv * w;

			numerator += bspline.points[i][j].second->pos * B;
			denominator += B;
		}
	}

	if (denominator == 0.0f) { return false; }

	outPoint = numerator / denominator;
	return true;
}

Mesh BSplineBuilder::CreateMesh(const STEPBSplineSurface& bspline, int uSegments, int vSegments)
{
	if (uSegments <= 0 || vSegments <= 0) { return Mesh(); }
	STEPUV<Vector<float>> expandKnots;
	expandKnots.u =  CreateExpandedKnots(bspline.multiple.u, bspline.knots.u);
	expandKnots.v = CreateExpandedKnots(bspline.multiple.v, bspline.knots.v);
	if (!CheckKnotSize(expandKnots.u, bspline.points.size(), bspline.degree.u)) { return Mesh(); }
	if (!CheckKnotSize(expandKnots.v, bspline.points[0].size(), bspline.degree.v)) { return Mesh(); }

	float uMin, uMax, vMin, vMax;
	if (!GetParameterRange(expandKnots.u, bspline.degree.u, uMin, uMax)) { return Mesh(); }
	if (!GetParameterRange(expandKnots.v, bspline.degree.v, vMin, vMax)) { return Mesh(); }

	const int columnCount = uSegments + 1;
	const int rowCount = vSegments + 1;

	// 格子点を先に作る
	Vector<Vector3> points;
	points.resize(columnCount * rowCount);
	auto gridIndex = [columnCount](int x, int y)
	{
		return y * columnCount + x;
	};

	for (int y = 0; y < rowCount; ++y) {
		const float tv = (float)y / (float)vSegments;
		const float v = vMin + (vMax - vMin) * tv;

		for (int x = 0; x < columnCount; ++x) {
			const float tu = (float)x / (float)uSegments;
			const float u = uMin + (uMax - uMin) * tu;

			Vector3 pos;
			if (!Evaluate(bspline, expandKnots, u, v, pos)) {
				pos = { 0.0f, 0.0f, 0.0f };
			}

			points[gridIndex(x, y)] = pos;
		}
	}

	// 1セルにつき2三角形
	Vector<Vector3> triangles;
	triangles.reserve(uSegments * vSegments * 6);

	for (int y = 0; y < vSegments; ++y) {
		for (int x = 0; x < uSegments; ++x) {
			const int i0 = gridIndex(x, y);
			const int i1 = gridIndex(x + 1, y);
			const int i2 = gridIndex(x, y + 1);
			const int i3 = gridIndex(x + 1, y + 1);

			// triangle 1: i0, i2, i1
			{
				triangles.push_back(points[i0]);
				triangles.push_back(points[i2]);
				triangles.push_back(points[i1]);
			}

			// triangle 2: i1, i2, i3
			{
				triangles.push_back(points[i1]);
				triangles.push_back(points[i2]);
				triangles.push_back(points[i3]);

			}
		}
	}

	return Mesh(std::move(triangles), Mesh::DrawType::Triangles);
}
}
}