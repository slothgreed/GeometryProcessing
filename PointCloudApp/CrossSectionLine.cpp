#include "CrossSectionLine.h"
#include "HalfEdgeNode.h"
#include "SimpleShader.h"
namespace KI
{

CrossSectionLine::CrossSectionLine()
{
	m_drawBuffer = std::make_unique<GLBuffer>();
	m_lineBuffer = std::make_unique<GLBuffer>();
	m_shader.Build();
}
void CrossSectionLine::Initialize(int lineNum)
{

	DrawArrayIndirect draw;
	draw.count = 0;
	draw.instanceCount = 1;
	draw.first = 0;
	draw.baseInstance = 0;
	if (!m_drawBuffer->Created()) {
		m_drawBuffer->Create(1, sizeof(DrawArrayIndirect));
	}
	m_drawBuffer->BufferSubData(0, sizeof(DrawArrayIndirect), &draw);
	if (m_lineBuffer->Num() != lineNum) {
		m_lineBuffer->Create(DATA_FLOAT, lineNum * 2, sizeof(Vector3), nullptr);
	}
}
void CrossSectionLine::Draw(HalfEdgeNode* pNode, const Vector4& plane, const DrawContext& context)
{
	Initialize(pNode->GetFaceIndexGpu()->Num() / 3);
	CrossSectionLine::Shader::Args args;
	args.posBuffer = pNode->GetPositionGpu();
	args.triBuffer = pNode->GetFaceIndexGpu();
	args.triNum = pNode->GetFaceIndexGpu()->Num() / 3;
	args.lineBuffer = m_lineBuffer.get();
	args.drawBuffer = m_drawBuffer.get();
	args.plane = plane;
	m_shader.Execute(args);

	auto pSimple = context.pResource->GetShaderTable()->GetSimpleShader();
	pSimple->Use();
	pSimple->SetModel(pNode->GetMatrix());
	pSimple->SetColor(Vector3(1, 0, 0));
	pSimple->SetCamera(context.pResource->GetCameraBuffer());
	pSimple->SetPosition(m_lineBuffer.get());
	pSimple->DrawArrayIndirectBuffer(GL_LINES, 0, m_drawBuffer.get());
}

void CrossSectionLine::ShowUI(HalfEdgeNode* pNode, UIContext& ui)
{
}


void CrossSectionLine::Shader::FetchUniformLocation()
{
	m_plane = GetUniformLocation("u_plane");
	m_triNum = GetUniformLocation("u_triNum");

}
ShaderPath CrossSectionLine::Shader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("common.h");
	path.shader[SHADER_PROGRAM_COMPUTE] = "algorithm/crossSectionLine.comp";
	return path;
}
void CrossSectionLine::Shader::Execute(const Args& args)
{
	Use();
	BindUniform(m_plane, args.plane);
	BindUniform(m_triNum, args.triNum);
	BindShaderStorage(0, args.posBuffer->Handle());
	BindShaderStorage(1, args.triBuffer->Handle());
	BindShaderStorage(2, args.lineBuffer->Handle());
	BindShaderStorage(3, args.drawBuffer->Handle());
	Dispatch(GetDispatchNum1D(args.triBuffer->Num()));
	BarrierAll();
	UnUse();
	Vector<Vector3> lineData(args.lineBuffer->Num());
	args.lineBuffer->GetBufferData(lineData);

}
}