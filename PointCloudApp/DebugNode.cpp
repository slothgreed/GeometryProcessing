#include "DebugNode.h"
#include "SimpleShader.h"
#include "DelaunayGenerator.h"
namespace KI
{
DebugNode::DebugNode(const String& name)
: RenderNode(name)
{
}
void DebugNode::SetPrimitive(const std::vector<Shared<Primitive>>& pPrimitive)
{
	m_pPrimitive = pPrimitive;
	BuildGLBuffer();
}

void DebugNode::BuildGLBuffer()
{
	if (m_pPrimitive.size() == 0) { return; }
	if (m_gpu.size() != m_pPrimitive.size()) {
		m_gpu.resize(m_pPrimitive.size());
	}

	for (int i = 0; i < m_pPrimitive.size(); i++) {
		if (!m_pPrimitive[i]->NeedUpdate()) { continue; }
		m_gpu[i].pPosition = std::make_unique<GLBuffer>();
		m_gpu[i].pPosition->Create(m_pPrimitive[i]->Position());

		if (m_pPrimitive[i]->Color().size() != 0) {
			m_gpu[i].pColor = std::make_unique<GLBuffer>();
			m_gpu[i].pColor->Create(m_pPrimitive[i]->Color());
		}

		m_pPrimitive[i]->ClearUpdate();
	}
}

void DebugNode::DrawNode(const DrawContext& context)
{
	BuildGLBuffer();
	if (m_gpu.size() == 0) { return; }
	const auto& pResource = context.pResource;
	auto pVertexColorShader = pResource->GetShaderTable()->GetVertexColorShader();
	pVertexColorShader->Use();
	pVertexColorShader->SetCamera(pResource->GetCameraBuffer());
	pVertexColorShader->SetModel(GetMatrix());
	for (int i = 0; i < m_gpu.size(); i++) {
		pVertexColorShader->SetPosition(m_gpu[i].pPosition.get());
		pVertexColorShader->SetColor(m_gpu[i].pColor.get());
		pVertexColorShader->DrawArray(m_pPrimitive[i]->GetType(), m_pPrimitive[i]->GetNum());
	}
}

DelaunayDebugNode::DelaunayDebugNode(const String& name)
	: DebugNode(name)
{
	BuildDebugPrimitive();
};
void DelaunayDebugNode::BuildDebugPrimitive()
{
	const float M_PI = 3.14f;
	m_pPrimitive.clear();
	if (m_ui.pattern == 0) {
		auto primitiveA = std::make_shared<Primitive>();
		Vector<Vector3> outer; Vector<Vector3> outerColor;
		outer.push_back(Vector3(-100, -100, 0)); outerColor.push_back(Vector3(1, 0, 0));
		outer.push_back(Vector3(100, -100, 0)); outerColor.push_back(Vector3(1, 0, 0));
		outer.push_back(Vector3(100, 100, 0)); outerColor.push_back(Vector3(1, 0, 0));
		outer.push_back(Vector3(-100, 100, 0)); outerColor.push_back(Vector3(1, 0, 0));
		primitiveA->SetPosition(std::move(outer));
		primitiveA->SetColor(std::move(outerColor));
		primitiveA->SetType(GL_LINE_LOOP);

		auto primitiveB = std::make_shared<Primitive>();
		Vector<Vector3> inner; Vector<Vector3> innerColor;
		inner.push_back(Vector3(-50, -50, 0)); innerColor.push_back(Vector3(0, 1, 0));
		inner.push_back(Vector3(50, -50, 0)); innerColor.push_back(Vector3(0, 1, 0));
		inner.push_back(Vector3(50, 50, 0)); innerColor.push_back(Vector3(0, 1, 0));
		inner.push_back(Vector3(-50, 50, 0)); innerColor.push_back(Vector3(0, 1, 0));
		primitiveB->SetPosition(std::move(inner));
		primitiveB->SetColor(std::move(innerColor));
		primitiveB->SetType(GL_LINE_LOOP);

		m_pPrimitive.push_back(primitiveA);
		m_pPrimitive.push_back(primitiveB);
	}

	if (m_ui.pattern == 1) {
		auto primitiveA = std::make_shared<Primitive>();
		Vector<Vector3> outer;
		Vector<Vector3> outerColor;

		// 外側：星型っぽい形を for 文で作成
		int outerCount = 10;
		float outerRadius1 = 100.0f; // 大きい半径
		float outerRadius2 = 60.0f;  // 小さい半径
		for (int i = 0; i < outerCount; i++) {
			float angle = i * (M_PI * 2.0f / outerCount);
			float radius = (i % 2 == 0) ? outerRadius1 : outerRadius2;
			outer.push_back(Vector3(radius * cos(angle), radius * sin(angle), 0));
			outerColor.push_back(Vector3(1, 0, 0)); // 赤
		}
		primitiveA->SetPosition(std::move(outer));
		primitiveA->SetColor(std::move(outerColor));
		primitiveA->SetType(GL_LINE_LOOP);


		// 内側：円形に近い多角形
		auto primitiveB = std::make_shared<Primitive>();
		Vector<Vector3> inner;
		Vector<Vector3> innerColor;

		int innerCount = 20;
		float innerRadius = 30.0f;
		for (int i = 0; i < innerCount; i++) {
			float angle = i * (M_PI * 2.0f / innerCount);
			inner.push_back(Vector3(innerRadius * cos(angle), innerRadius * sin(angle), 0));
			innerColor.push_back(Vector3(0, 1, 0)); // 緑
		}
		primitiveB->SetPosition(std::move(inner));
		primitiveB->SetColor(std::move(innerColor));
		primitiveB->SetType(GL_LINE_LOOP);


		m_pPrimitive.push_back(primitiveA);
		m_pPrimitive.push_back(primitiveB);
	}

	if (m_ui.pattern == 2) {
		// 外側：六角形
		auto primitiveA = std::make_shared<Primitive>();
		Vector<Vector3> outerA;
		Vector<Vector3> outerColorA;

		int outerCountA = 6;
		float radiusA = 100.0f;
		for (int i = 0; i < outerCountA; i++) {
			float angle = i * (M_PI * 2.0f / outerCountA);
			outerA.push_back(Vector3(radiusA * cos(angle), radiusA * sin(angle), 0));
			outerColorA.push_back(Vector3(0, 0, 1)); // 青
		}
		primitiveA->SetPosition(std::move(outerA));
		primitiveA->SetColor(std::move(outerColorA));
		primitiveA->SetType(GL_LINE_LOOP);

		// 内側：三角形
		auto primitiveB = std::make_shared<Primitive>();
		Vector<Vector3> innerB;
		Vector<Vector3> innerColorB;

		int innerCountB = 3;
		float innerRadiusB = 40.0f;
		for (int i = 0; i < innerCountB; i++) {
			float angle = i * (M_PI * 2.0f / innerCountB) + M_PI / 6.0f; // ちょっと回転
			innerB.push_back(Vector3(innerRadiusB * cos(angle), innerRadiusB * sin(angle), 0));
			innerColorB.push_back(Vector3(1, 1, 0)); // 黄
		}
		primitiveB->SetPosition(std::move(innerB));
		primitiveB->SetColor(std::move(innerColorB));
		primitiveB->SetType(GL_LINE_LOOP);

		m_pPrimitive.push_back(primitiveA);
		m_pPrimitive.push_back(primitiveB);
	}

	if (m_ui.pattern == 3) {
		// 外側：四角形
		auto primitiveC = std::make_shared<Primitive>();
		Vector<Vector3> outerC;
		Vector<Vector3> outerColorC;

		outerC.push_back(Vector3(-120, -120, 0));
		outerC.push_back(Vector3(120, -120, 0));
		outerC.push_back(Vector3(120, 120, 0));
		outerC.push_back(Vector3(-120, 120, 0));
		for (int i = 0; i < 4; i++) outerColorC.push_back(Vector3(1, 0.5, 0)); // オレンジ
		primitiveC->SetPosition(std::move(outerC));
		primitiveC->SetColor(std::move(outerColorC));
		primitiveC->SetType(GL_LINE_LOOP);


		// 内側１：円形（左）
		auto primitiveD1 = std::make_shared<Primitive>();
		Vector<Vector3> innerD1;
		Vector<Vector3> innerColorD1;

		int circleCount = 24;
		float circleRadius = 30.0f;
		for (int i = 0; i < circleCount; i++) {
			float angle = i * (M_PI * 2.0f / circleCount);
			innerD1.push_back(Vector3(-50 + circleRadius * cos(angle), 0 + circleRadius * sin(angle), 0));
			innerColorD1.push_back(Vector3(0, 1, 1)); // シアン
		}
		primitiveD1->SetPosition(std::move(innerD1));
		primitiveD1->SetColor(std::move(innerColorD1));
		primitiveD1->SetType(GL_LINE_LOOP);


		// 内側２：五角形（右）
		auto primitiveD2 = std::make_shared<Primitive>();
		Vector<Vector3> innerD2;
		Vector<Vector3> innerColorD2;

		int pentagonCount = 5;
		float pentagonRadius = 35.0f;
		for (int i = 0; i < pentagonCount; i++) {
			float angle = i * (M_PI * 2.0f / pentagonCount) - M_PI / 2.0f;
			innerD2.push_back(Vector3(60 + pentagonRadius * cos(angle), 0 + pentagonRadius * sin(angle), 0));
			innerColorD2.push_back(Vector3(1, 0, 1)); // マゼンタ
		}
		primitiveD2->SetPosition(std::move(innerD2));
		primitiveD2->SetColor(std::move(innerColorD2));
		primitiveD2->SetType(GL_LINE_LOOP);

		m_pPrimitive.push_back(primitiveC);
		m_pPrimitive.push_back(primitiveD1);
		m_pPrimitive.push_back(primitiveD2);
	}

	if (m_ui.pattern == 4) {
		// 外側：凹型のポリゴン（くの字型）
		auto primitive1 = std::make_shared<Primitive>();
		Vector<Vector3> outer1;
		Vector<Vector3> outerColor1;

		outer1.push_back(Vector3(-100, -100, 0));
		outer1.push_back(Vector3(100, -100, 0));
		outer1.push_back(Vector3(100, 0, 0));
		outer1.push_back(Vector3(0, 0, 0));
		outer1.push_back(Vector3(0, 100, 0));
		outer1.push_back(Vector3(-100, 100, 0));

		for (int i = 0; i < (int)outer1.size(); i++) outerColor1.push_back(Vector3(1, 0, 0));
		primitive1->SetPosition(std::move(outer1));
		primitive1->SetColor(std::move(outerColor1));
		primitive1->SetType(GL_LINE_LOOP);

		m_pPrimitive.push_back(primitive1);
	}


	if (m_ui.pattern == 5) {
		// 外側：四角形
		auto primitive3 = std::make_shared<Primitive>();
		Vector<Vector3> outer3;
		Vector<Vector3> outerColor3;

		outer3.push_back(Vector3(-100, -100, 0));
		outer3.push_back(Vector3(100, -100, 0));
		outer3.push_back(Vector3(100, 100, 0));
		outer3.push_back(Vector3(-100, 100, 0));
		for (int i = 0; i < 4; i++) outerColor3.push_back(Vector3(1, 0.5, 0));
		primitive3->SetPosition(std::move(outer3));
		primitive3->SetColor(std::move(outerColor3));
		primitive3->SetType(GL_LINE_LOOP);

		// 内側：外周に接している円
		auto primitive3_hole = std::make_shared<Primitive>();
		Vector<Vector3> inner3;
		Vector<Vector3> innerColor3;

		int holeCount3 = 20;
		float radius3 = 30.0f;
		for (int i = 0; i < holeCount3; i++) {
			float angle = i * (M_PI * 2.0f / holeCount3);
			// 中心を右端に寄せる（外周に接触）
			inner3.push_back(Vector3(70 + radius3 * cos(angle), 0 + radius3 * sin(angle), 0));
			innerColor3.push_back(Vector3(0, 1, 1));
		}
		primitive3_hole->SetPosition(std::move(inner3));
		primitive3_hole->SetColor(std::move(innerColor3));
		primitive3_hole->SetType(GL_LINE_LOOP);

		m_pPrimitive.push_back(primitive3);
		m_pPrimitive.push_back(primitive3_hole);
	}

	if (m_ui.pattern == 6) {
		// 外側：大きな円形
		auto primitive4 = std::make_shared<Primitive>();
		Vector<Vector3> outer4;
		Vector<Vector3> outerColor4;

		int outerCount4 = 40;
		float outerRadius4 = 200.0f;
		for (int i = 0; i < outerCount4; i++) {
			float angle = i * (M_PI * 2.0f / outerCount4);
			outer4.push_back(Vector3(outerRadius4 * cos(angle), outerRadius4 * sin(angle), 0));
			outerColor4.push_back(Vector3(1, 0, 1));
		}
		outer4.push_back(Vector3(0, 0, 0));
		outerColor4.push_back(Vector3(1, 0, 1));

		primitive4->SetPosition(std::move(outer4));
		primitive4->SetColor(std::move(outerColor4));
		primitive4->SetType(GL_LINE_LOOP);

		// 内側：極小円
		auto primitive4_hole = std::make_shared<Primitive>();
		Vector<Vector3> inner4;
		Vector<Vector3> innerColor4;

		int innerCount4 = 12;
		float innerRadius4 = 5.0f;
		for (int i = 0; i < innerCount4; i++) {
			float angle = i * (M_PI * 2.0f / innerCount4);
			inner4.push_back(Vector3(innerRadius4 * cos(angle), innerRadius4 * sin(angle), 0));
			innerColor4.push_back(Vector3(0, 1, 0));
		}
		primitive4_hole->SetPosition(std::move(inner4));
		primitive4_hole->SetColor(std::move(innerColor4));
		primitive4_hole->SetType(GL_LINE_LOOP);

		m_pPrimitive.push_back(primitive4);
		m_pPrimitive.push_back(primitive4_hole);
	}

	m_Delaunay.Clear();
	m_Delaunay.SetTarget(&m_pPrimitive[0]->Position());
	for (size_t i = 1; i < m_pPrimitive.size(); i++) {
		m_Delaunay.AddInner(&m_pPrimitive[i]->Position());
	}

	DebugNode::SetPrimitive(m_pPrimitive);
}
void DelaunayDebugNode::ShowUI(UIContext& ui)
{
	if (ImGui::SliderInt("DebugType", &m_ui.pattern, 0, 6)) {
		BuildDebugPrimitive();
	}
	m_Delaunay.ShowUI(this, ui);
}
}