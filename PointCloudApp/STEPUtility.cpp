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
		Assert::Failed();
	}

	return Polyline();
}

bool BSplineBuilder::CheckKnotSize(const Vector<float>& knots, int pointNum, int degree)
{
	return (knots.size() == pointNum + degree + 1);
}


Vector<float> BSplineBuilder::CreateExpandedKnots(const Vector<int>& multiple, const Vector<float>& knots)
{
	if (multiple.size() != knots.size()) { return Vector<float>(); }

	Vector<float> outExpandKnots;
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


float BSplineBuilder::Basis(int j, int k, float t, const Vector<float>& knotsT)
{
	if (k == 0) {
		if (knotsT[j] <= t && t < knotsT[j + 1]) {
			return 1.0f;
		}
		if (MathHelper::IsSame(t, knotsT.back())) {
			return (knotsT[j] < t && MathHelper::IsSame(knotsT[j + 1], t)) ? 1.0f : 0.0f;
		}
		return 0.0f;
	}

	float left = 0.0f;
	float right = 0.0f;

	const float leftDenom = knotsT[j + k] - knotsT[j];
	if (leftDenom != 0.0f) {
		left = (t - knotsT[j]) / leftDenom * Basis(j, k - 1, t, knotsT);
	}

	const float rightDenom = knotsT[j + k + 1] - knotsT[j + 1];
	if (rightDenom != 0.0f) {
		right = (knotsT[j + k + 1] - t) / rightDenom * Basis(j + 1, k - 1, t, knotsT);
	}

	return left + right;
}

bool BSplineBuilder::Evaluate(const STEPBSplineCurve& bspline, float u, Vector3& outPoint)
{
	const bool hasWeight = !bspline.rational.empty();
	if (hasWeight && bspline.rational.size() != bspline.points.size()) { return false; }

	Vector3 numerator{ 0.0f, 0.0f, 0.0f };
	float denominator = 0.0f;

	auto clamedU = u;
	clamedU = std::max(bspline.expandKnots.front(), clamedU);
	clamedU = std::min(bspline.expandKnots.back(), clamedU);

	for (int i = 0; i < bspline.points.size(); ++i) {
		const float N = Basis(i, bspline.degree, clamedU, bspline.expandKnots);
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


bool BSplineBuilder::Evaluate(const STEPBSplineSurfaceBase& bspline, const Vector2& uv, Vector3& outPoint)
{
	Vector3 numerator{ 0.0f, 0.0f, 0.0f };
	float denominator = 0.0f;

	auto clampedUV = uv;
	clampedUV.x = std::max(bspline.expandKnots.u.front(), clampedUV.x);
	clampedUV.x = std::min(bspline.expandKnots.u.back(), clampedUV.x);
	clampedUV.y = std::max(bspline.expandKnots.v.front(), clampedUV.y);
	clampedUV.y = std::min(bspline.expandKnots.v.back(), clampedUV.y);

	const bool hasWeight = !bspline.rational.empty();
	for (int i = 0; i < bspline.GetUNum(); ++i) {
		const float Nu = Basis(i, bspline.degree.u, clampedUV.x, bspline.expandKnots.u);
		if (Nu == 0.0f) { continue; }

		for (int j = 0; j < bspline.GetVNum(); ++j) {
			const float Nv = Basis(j, bspline.degree.v, clampedUV.y, bspline.expandKnots.v);
			if (Nv == 0.0f) { continue; }

			const float w = hasWeight ? bspline.rational[i][j] : 1.0f;
			const float B = Nu * Nv * w;

			numerator += bspline.points[i][j].second->pos * B;
			denominator += B;
		}
	}

	if (denominator == 0.0f) {
		return false;
	}

	outPoint = numerator / denominator;
	return true;
}

bool BSplineBuilder::ProjectPointToUV(const STEPBSplineSurfaceBase& bspline, const Vector3& target, Vector2& uv)
{
	const float offset = 1e-4f;
	const int maxIter = 20;

	float uMin = bspline.knots.u.front();
	float uMax = bspline.knots.u.back();
	float vMin = bspline.knots.v.front();
	float vMax = bspline.knots.v.back();

	uv = Vector2(uMin + uMax, vMin + vMax) * 0.5f; // 暫定値

	for (int iter = 0; iter < maxIter; ++iter) {
		Vector3 S;
		if (!Evaluate(bspline, uv, S)) {
			Assert::Failed();
			continue;
		}

		Vector3 SxMax;
		if(!Evaluate(bspline, Vector2(std::min(uv.x + offset, uMax), uv.y), SxMax)){
			Assert::Failed();
			continue;
		}
		Vector3 SxMin;
		if (!Evaluate(bspline, Vector2(std::max(uv.x - offset, uMin), uv.y), SxMin)) {
			Assert::Failed();
			continue;
		}
		// U方向の傾きを求める
		auto Su = (SxMax -	SxMin) / (std::min(uv.x + offset, uMax) - std::max(uv.x - offset, uMin));

		Vector3 SyMax;
		if(!Evaluate(bspline, Vector2(uv.x, std::min(uv.y + offset, vMax)), SyMax)){
			Assert::Failed();
			continue;
		}

		Vector3 SyMin;
		if (!Evaluate(bspline, Vector2(uv.x, std::max(uv.y - offset, vMin)), SyMin)) {
			Assert::Failed();
			continue;
		}
		// V方向の傾きを求める
		auto Sv = (SyMax - SyMin) / (std::min(uv.y + offset, vMax) - std::max(uv.y - offset, vMin));

		// 最小二乗法で、targetに近づくようにuvを更新する。
		auto r = target - S;

		float a00 = glm::dot(Su, Su);
		float a01 = glm::dot(Su, Sv);
		float a11 = glm::dot(Sv, Sv);

		float b0 = glm::dot(Su, r);
		float b1 = glm::dot(Sv, r);

		float det = a00 * a11 - a01 * a01;
		if (std::abs(det) < 1e-10f)
			return false;

		float du = (b0 * a11 - b1 * a01) / det;
		float dv = (a00 * b1 - a01 * b0) / det;

		uv.x += du;
		uv.y += dv;

		uv.x = std::clamp(uv.x, uMin, uMax);
		uv.y = std::clamp(uv.y, vMin, vMax);

		// 更新量が十分小さければ終了
		if (std::abs(du) < MathHelper::EPS &&
			std::abs(dv) < MathHelper::EPS) {
			return true;
		}
	}

	return false;
}

Mesh BSplineBuilder::CreateMesh(const STEPBSplineSurfaceBase& bspline, int uSegments, int vSegments)
{
	if (uSegments <= 0 || vSegments <= 0) { return Mesh(); }
	if (!CheckKnotSize(bspline.expandKnots.u, bspline.points.size(), bspline.degree.u)) { return Mesh(); }
	if (!CheckKnotSize(bspline.expandKnots.v, bspline.points[0].size(), bspline.degree.v)) { return Mesh(); }

	float uMin, uMax, vMin, vMax;
	if (!GetParameterRange(bspline.expandKnots.u, bspline.degree.u, uMin, uMax)) { return Mesh(); }
	if (!GetParameterRange(bspline.expandKnots.v, bspline.degree.v, vMin, vMax)) { return Mesh(); }

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
			if (!Evaluate(bspline, Vector2(u, v), pos)) {
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