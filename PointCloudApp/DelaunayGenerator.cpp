#include "DelaunayGenerator.h"
#include "PrimitiveNode.h"
#include "PointCloudNode.h"
#include "PointCloud.h"
#include "Primitives.h"
#include "Utility.h"
namespace KI
{


DelaunayGenerator::DelaunayGenerator(PointCloudNode* pPointCloud)
	:m_pPointCloud(pPointCloud)
{
}

DelaunayGenerator::~DelaunayGenerator()
{

}

Vector<unsigned int> DelaunayGenerator::Execute2D(const Vector<Vector3>& points)
{
	m_HugeTriangle = CreateHugeTriangle2D(points);
	return Vector<unsigned int>();
}

DelaunayGenerator::Triangle DelaunayGenerator::CreateHugeTriangle2D(const Vector<Vector3>& points)
{
	Vector3 center = Vector3(0);
	for (const auto& p : points) {
		center += p;
	}

	center /= points.size();

	float maxLength = 0;
	Vector3 maxPos;
	for (const auto& p : points) {
		auto len = glm::length2(p - center);
		if (maxLength < len) {
			maxLength = len;
			maxPos = p;
		}
	}

	// �~�𕢂����O�p�`���\�z
	Triangle tri;
	Vector3 radDir = center - maxPos;
	float radius = glm::length(radDir) * 1.01f;
	float root3 = sqrtf(radius);
	tri.pos0 = center + (radius * radDir);
	tri.pos1 = maxPos + (root3 * Vector3(radDir.x, -radDir.y, 0.0));
	tri.pos2 = maxPos - (root3 * Vector3(radDir.x, -radDir.y, 0.0));

	return tri;
}

DelaunayGenerator::Circumscribe DelaunayGenerator::CalcCircumscribedCircle(const Triangle& triangle)
{
	const auto& pos1 = triangle.pos0;
	const auto& pos2 = triangle.pos1;
	const auto& pos3 = triangle.pos2;


	// �O�p�`�̊e�ӂ̒������v�Z
	float dA = glm::length2(pos1);
	float dB = glm::length2(pos2);
	float dC = glm::length2(pos3);

	// �s�񎮂��g���ĊO�S�̍��W�����߂�
	float aux1 = (dA * (pos3.y - pos2.y) + dB * (pos1.y - pos3.y) + dC * (pos2.y - pos1.y));
	float aux2 = -(dA * (pos3.x - pos2.x) + dB * (pos1.x - pos3.x) + dC * (pos2.x - pos1.x));
	float div = (2 * (pos1.x * (pos3.y - pos2.y) + pos2.x * (pos1.y - pos3.y) + pos3.x * (pos2.y - pos1.y)));

	Circumscribe c;
	// �O�S��x���W��y���W
	c.center = Vector3(aux1 / div, aux2 / div, 0.0f);
	// ���a���v�Z�i�O�S����C�ӂ̒��_�܂ł̋����j
	c.radius = glm::length(c.center - pos1);
	return  c;

}

// 3�_�̍��W����͂Ƃ��āA�O�ډ~�̒��S�Ɣ��a���v�Z����֐�
void circumcircle(float x1, float y1, float x2, float y2, float x3, float y3, float& cx, float& cy, float& radius)
{
	// �O�p�`�̊e�ӂ̒������v�Z
	float dA = x1 * x1 + y1 * y1;
	float dB = x2 * x2 + y2 * y2;
	float dC = x3 * x3 + y3 * y3;

	// �s�񎮂��g���ĊO�S�̍��W�����߂�
	float aux1 = (dA * (y3 - y2) + dB * (y1 - y3) + dC * (y2 - y1));
	float aux2 = -(dA * (x3 - x2) + dB * (x1 - x3) + dC * (x2 - x1));
	float div = (2 * (x1 * (y3 - y2) + x2 * (y1 - y3) + x3 * (y2 - y1)));

	// �O�S��x���W��y���W
	cx = aux1 / div;
	cy = aux2 / div;

	// ���a���v�Z�i�O�S����C�ӂ̒��_�܂ł̋����j
	radius = sqrt((cx - x1) * (cx - x1) + (cy - y1) * (cy - y1));
}

void DelaunayGenerator::ShowUI()
{
	if (ImGui::Button("DelaunayGenerator")) {
		Execute2D(m_pPointCloud->GetData()->Position());
		auto pTriangle = std::make_shared<Primitive>();
		Vector<Vector3> pos;
		pos.push_back(m_HugeTriangle.pos0);
		pos.push_back(m_HugeTriangle.pos1);
		pos.push_back(m_HugeTriangle.pos2);
		pTriangle->SetPosition(std::move(pos));
		pTriangle->SetType(GL_TRIANGLES);
		m_pPointCloud->AddNode(std::make_shared<PrimitiveNode>("DelaunayTriangle", pTriangle, ColorUtility::CreatePrimary(4)));

		auto c = CalcCircumscribedCircle(m_HugeTriangle);
		Shared<Primitive> pCircle = std::make_shared<Circle>(c.radius, c.center);
		m_pPointCloud->AddNode(std::make_shared<PrimitiveNode>("DelaunayCircle", pCircle, ColorUtility::CreatePrimary(3)));
	}
}
}