#include "CrossSectionFill.h"
#include "HalfEdgeNode.h"
#include "Primitives.h"
namespace KI
{

CrossSectionFill::CrossSectionFill()
{
	m_shader.Build();
	m_planePosition = std::make_unique<GLBuffer>();
	m_planeIndex = std::make_unique<GLBuffer>();
}

void CrossSectionFill::Draw(HalfEdgeNode* pNode, const Vector4& plane, const DrawContext& context)
{
	auto gl = context.pResource->GL();
	UpdatePlane(pNode, plane);

	// Count the clipped mesh's front and back faces in stencil.
	gl->EnableClipDistance(0);
	gl->SetStencilMask(0xFF);
	gl->SetClearStencil(0);
	gl->Clear(GL_STENCIL_BUFFER_BIT);
	gl->EnableStencil();
	gl->SetStencilFunc(GL_ALWAYS, 0, 0xFF);
	gl->SetStencilOperationSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
	gl->SetStencilOperationSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_DECR_WRAP);
	gl->ColorMask(false);
	gl->DepthMask(false);
	gl->DisableDepth();
	gl->DisableCullFace();
	DrawMesh(pNode, plane, Vector3(0.0f), context);
	gl->DisableClipDistance(0);

	// The cutting plane is colored only where the winding count is non-zero.
	gl->ColorMask(true);
	gl->EnableDepth();
	gl->DepthMask(true);
	gl->SetStencilMask(0x00);
	gl->SetStencilFunc(GL_NOTEQUAL, 0, 0xFF);
	gl->SetStencilOperation(GL_KEEP, GL_KEEP, GL_KEEP);
	m_shader.Use();
	m_shader.SetCamera(context.pResource->GetCameraBuffer());
	m_shader.SetModel(pNode->GetMatrix());
	m_shader.SetPlane(Vector4(0.0f));
	m_shader.SetColor(Vector3(1.0f, 0.15f, 0.1f));
	m_shader.SetPosition(m_planePosition.get());
	m_shader.DrawElement(GL_TRIANGLES, m_planeIndex.get());

	// Restore the state expected by the forward pass.
	gl->DepthMask(true);
	gl->SetStencilMask(0xFF);
	gl->SetStencilFunc(GL_ALWAYS, 0, 0xFF);
	gl->SetStencilOperation(GL_KEEP, GL_KEEP, GL_KEEP);
	gl->DisableStencil();
	gl->EnableCullFace();
}

void CrossSectionFill::ShowUI(HalfEdgeNode* pNode, UIContext& ui)
{
}

void CrossSectionFill::UpdatePlane(const HalfEdgeNode* pNode, const Vector4& plane)
{
	PlanePrimitive::Axis axis = PlanePrimitive::X;
	float position = -plane.w / plane.x;
	if (std::abs(plane.y) > std::abs(plane.x) && std::abs(plane.y) >= std::abs(plane.z)) {
		axis = PlanePrimitive::Y;
		position = -plane.w / plane.y;
	} else if (std::abs(plane.z) > std::abs(plane.x) && std::abs(plane.z) > std::abs(plane.y)) {
		axis = PlanePrimitive::Z;
		position = -plane.w / plane.z;
	}

	auto primitive = PlanePrimitive(
		pNode->GetBoundBox().Min(), pNode->GetBoundBox().Max(), position, axis);
	if (!m_planePosition->Created()) {
		m_planePosition->Create(primitive.Position());
		m_planeIndex->Create(primitive.Index());
	} else {
		m_planePosition->BufferSubData(0, primitive.Position());
	}
}

void CrossSectionFill::DrawMesh(HalfEdgeNode* pNode, const Vector4& plane, const Vector3& color, const DrawContext& context)
{
	m_shader.Use();
	m_shader.SetCamera(context.pResource->GetCameraBuffer());
	m_shader.SetModel(pNode->GetMatrix());
	m_shader.SetPlane(plane);
	m_shader.SetColor(color);
	m_shader.SetPosition(pNode->GetPositionGpu());
	m_shader.DrawElement(GL_TRIANGLES, pNode->GetFaceIndexGpu());
}

ShaderPath CrossSectionFill::Shader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("common.h");
	path.shader[SHADER_PROGRAM_VERTEX] = "algorithm/crossSectionFill.vert";
	path.shader[SHADER_PROGRAM_FRAG] = "algorithm/crossSectionFill.frag";
	return path;
}

void CrossSectionFill::Shader::FetchUniformLocation()
{
	m_model = GetUniformLocation("u_Model");
	m_color = GetUniformLocation("u_Color");
	m_plane = GetUniformLocation("u_Plane");
}

void CrossSectionFill::Shader::SetModel(const Matrix4x4& value)
{
	BindUniform(m_model, value);
}

void CrossSectionFill::Shader::SetCamera(const GLBuffer* pBuffer)
{
	BindShaderStorage(0, pBuffer->Handle());
}

void CrossSectionFill::Shader::SetPosition(const GLBuffer* pBuffer)
{
	SetVertexFormat(VertexFormat(ATTRIB_POSITION, pBuffer));
	glBindVertexBuffer(ATTRIB_POSITION, pBuffer->Handle(), 0, pBuffer->SizeOfData());
}

void CrossSectionFill::Shader::SetColor(const Vector3& value)
{
	BindUniform(m_color, value);
}

void CrossSectionFill::Shader::SetPlane(const Vector4& value)
{
	BindUniform(m_plane, value);
}

}
