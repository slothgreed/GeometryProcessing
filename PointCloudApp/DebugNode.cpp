#include "DebugNode.h"
#include "SimpleShader.h"
#include "DelaunayGenerator.h"
namespace KI
{
DebugNode::DebugNode(const String& name)
: RenderNode(name)
{
}

void DebugNode::SetPrimitive(const Shared<Primitive>& pPrimitive)
{
	m_pPrimitive.push_back(pPrimitive);
	BuildGLBuffer();
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
	for (int i = 0; i < m_gpu.size(); i++) {
		if (m_gpu[i].pColor) {
			auto pVertexColorShader = pResource->GetShaderTable()->GetVertexColorShader();
			pVertexColorShader->Use();
			pVertexColorShader->SetCamera(pResource->GetCameraBuffer());
			pVertexColorShader->SetModel(GetMatrix());
			pVertexColorShader->SetPosition(m_gpu[i].pPosition.get());
			pVertexColorShader->SetColor(m_gpu[i].pColor.get());
			pVertexColorShader->DrawArray(m_pPrimitive[i]->GetType(), m_pPrimitive[i]->GetNum());
		} else {
			auto pSimpleShader = pResource->GetShaderTable()->GetSimpleShader();
			pSimpleShader->Use();
			pSimpleShader->SetCamera(pResource->GetCameraBuffer());
			pSimpleShader->SetModel(GetMatrix());
			pSimpleShader->SetPosition(m_gpu[i].pPosition.get());
			pSimpleShader->SetColor(Vector3(0, 0, 1));
			pSimpleShader->DrawArray(m_pPrimitive[i]->GetType(), m_pPrimitive[i]->GetNum());
		}
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

		primitive4->SetPosition(std::move(outer4));
		primitive4->SetColor(std::move(outerColor4));
		primitive4->SetType(GL_LINE_LOOP);

		// 内側：極小円
		auto primitive4_hole = std::make_shared<Primitive>();
		Vector<Vector3> inner4;
		Vector<Vector3> innerColor4;

		int innerCount4 = 36;
		float innerRadius4 = 175.0f;
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
	} else if (m_ui.pattern == 6) {
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

		primitive4->SetPosition(std::move(outer4));
		primitive4->SetColor(std::move(outerColor4));
		primitive4->SetType(GL_LINE_LOOP);

		// 内側：極小円
		auto primitive4_hole = std::make_shared<Primitive>();
		Vector<Vector3> inner4;
		Vector<Vector3> innerColor4;

		int innerCount4 = 36;
		float innerRadius4 = 175.0f;
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
	} else if (m_ui.pattern == 7) {
		std::vector<glm::vec3> target = {
			glm::vec3(47.8937f, 0.0f, 0.0f),
			glm::vec3(47.1661f, 8.31664f, 3.63532e-07f),
			glm::vec3(45.0053f, 16.3806f, 7.16018e-07f),
			glm::vec3(41.4771f, 23.9468f, 1.04675e-06f),
			glm::vec3(36.6887f, 30.7854f, 1.34567e-06f),
			glm::vec3(30.7855f, 36.6887f, 1.60371e-06f),
			glm::vec3(23.9469f, 41.4771f, 1.81302e-06f),
			glm::vec3(16.3806f, 45.0053f, 1.96724e-06f),
			glm::vec3(8.3167f, 47.166f, 2.06169e-06f),
			glm::vec3(6.07096e-05f, 47.8937f, 2.0935e-06f),
			glm::vec3(-8.31659f, 47.1661f, 2.06169e-06f),
			glm::vec3(-16.3805f, 45.0054f, 1.96725e-06f),
			glm::vec3(-23.9468f, 41.4772f, 1.81302e-06f),
			glm::vec3(-30.7854f, 36.6887f, 1.60372e-06f),
			glm::vec3(-36.6886f, 30.7855f, 1.34568e-06f),
			glm::vec3(-41.4771f, 23.9469f, 1.04675e-06f),
			glm::vec3(-45.0053f, 16.3807f, 7.16023e-07f),
			glm::vec3(-47.166f, 8.31677f, 3.63537e-07f),
			glm::vec3(-47.8937f, 0.000121419f, 5.3074e-12f),
			glm::vec3(-47.1661f, -8.31653f, -3.63527e-07f),
			glm::vec3(-45.0054f, -16.3805f, -7.16014e-07f),
			glm::vec3(-41.4772f, -23.9467f, -1.04674e-06f),
			glm::vec3(-36.6888f, -30.7853f, -1.34567e-06f),
			glm::vec3(-30.7856f, -36.6886f, -1.60371e-06f),
			glm::vec3(-23.947f, -41.4771f, -1.81302e-06f),
			glm::vec3(-16.3808f, -45.0053f, -1.96724e-06f),
			glm::vec3(-8.31683f, -47.166f, -2.06169e-06f),
			glm::vec3(-0.000182129f, -47.8937f, -2.0935e-06f),
			glm::vec3(8.31647f, -47.1661f, -2.0617e-06f),
			glm::vec3(16.3804f, -45.0054f, -1.96725e-06f),
			glm::vec3(23.9467f, -41.4772f, -1.81303e-06f),
			glm::vec3(30.7853f, -36.6888f, -1.60372e-06f),
			glm::vec3(36.6885f, -30.7856f, -1.34568e-06f),
			glm::vec3(41.477f, -23.947f, -1.04676e-06f),
			glm::vec3(45.0052f, -16.3808f, -7.16029e-07f),
			glm::vec3(47.166f, -8.31689f, -3.63543e-07f)
		};

		std::vector<glm::vec3> inner = {
			glm::vec3(-11.6545f, 3.94161f, -1.72293e-07f),
			glm::vec3(-14.2592f, 3.71373f, -1.62332e-07f),
			glm::vec3(-16.7848f, 3.037f, -1.32752e-07f),
			glm::vec3(-19.1545f, 1.932f, -8.44502e-08f),
			glm::vec3(-21.2963f, 0.432283f, -1.88957e-08f),
			glm::vec3(-23.1451f, -1.41657f, 6.19201e-08f),
			glm::vec3(-24.6449f, -3.55838f, 1.55542e-07f),
			glm::vec3(-25.7499f, -5.92807f, 2.59124e-07f),
			glm::vec3(-26.4266f, -8.45365f, 3.69521e-07f),
			glm::vec3(-26.6545f, -11.0584f, 4.83377e-07f),
			glm::vec3(-26.4266f, -13.6631f, 5.97233e-07f),
			glm::vec3(-25.7499f, -16.1887f, 7.07629e-07f),
			glm::vec3(-24.6449f, -18.5584f, 8.11212e-07f),
			glm::vec3(-23.1452f, -20.7002f, 9.04834e-07f),
			glm::vec3(-21.2963f, -22.549f, 9.8565e-07f),
			glm::vec3(-19.1545f, -24.0488f, 1.0512e-06f),
			glm::vec3(-16.7848f, -25.1538f, 1.09951e-06f),
			glm::vec3(-14.2592f, -25.8305f, 1.12909e-06f),
			glm::vec3(-11.6545f, -26.0584f, 1.13905e-06f),
			glm::vec3(-9.0498f, -25.8305f, 1.12909e-06f),
			glm::vec3(-6.52422f, -25.1538f, 1.09951e-06f),
			glm::vec3(-4.15452f, -24.0488f, 1.05121e-06f),
			glm::vec3(-2.01271f, -22.5491f, 9.85652e-07f),
			glm::vec3(-0.163848f, -20.7002f, 9.04836e-07f),
			glm::vec3(1.33587f, -18.5584f, 8.11215e-07f),
			glm::vec3(2.44089f, -16.1887f, 7.07632e-07f),
			glm::vec3(3.11762f, -13.6632f, 5.97236e-07f),
			glm::vec3(3.34552f, -11.0584f, 4.8338e-07f),
			glm::vec3(3.11764f, -8.45372f, 3.69524e-07f),
			glm::vec3(2.44093f, -5.92814f, 2.59127e-07f),
			glm::vec3(1.33593f, -3.55845f, 1.55545e-07f),
			glm::vec3(-0.163774f, -1.41663f, 6.19226e-08f),
			glm::vec3(-2.01262f, 0.432235f, -1.88936e-08f),
			glm::vec3(-4.15443f, 1.93196f, -8.44486e-08f),
			glm::vec3(-6.52411f, 3.03697f, -1.3275e-07f),
			glm::vec3(-9.04969f, 3.71371f, -1.62332e-07f)
		};

		Vector<Vector3> targetColor(target.size());
		for (int i = 0; i < target.size(); i++) {
			targetColor.push_back(Vector3(0, 1, 0));
		}

		Vector<Vector3> innerColor(inner.size());
		for (int i = 0; i < inner.size(); i++) {
			innerColor.push_back(Vector3(0, 1, 0));
		}

		auto pTarget = std::make_shared<Primitive>();
		pTarget->SetPosition(std::move(target));
		pTarget->SetColor(std::move(targetColor));
		pTarget->SetType(GL_LINE_LOOP);

		m_pPrimitive.push_back(pTarget);

		auto pInner = std::make_shared<Primitive>();
		pInner->SetPosition(std::move(inner));
		pInner->SetColor(std::move(innerColor));
		pInner->SetType(GL_LINE_LOOP);
		m_pPrimitive.push_back(pInner);
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
	if (ImGui::SliderInt("DebugType", &m_ui.pattern, 0, 7)) {
		BuildDebugPrimitive();
	}
	m_Delaunay.ShowUI(this, ui);
}



GridNode::GridNode(const String& name, const Vector3& min, const Vector3& max, float interval)
	: DebugNode(name)
	, m_min(min)
	, m_max(max)
	, m_interval(interval)
{
	m_pPrimitive = std::make_shared<Primitive>();
	Vector<Vector3> positions;

	// X方向に平行な線 (Yが固定)
	for (float y = min.y; y <= max.y + 0.001f; y += interval) {
		positions.push_back(Vector3(min.x, y, min.z));
		positions.push_back(Vector3(max.x, y, min.z));
	}

	// Y方向に平行な線 (Xが固定)
	for (float x = min.x; x <= max.x + 0.001f; x += interval) {
		positions.push_back(Vector3(x, min.y, min.z));
		positions.push_back(Vector3(x, max.y, min.z));
	}

	m_pPrimitive->SetPosition(std::move(positions));
	m_pPrimitive->SetType(GL_LINES);
	DebugNode::SetPrimitive(m_pPrimitive);
}
}