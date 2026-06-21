#include "GaussianSplattingNode.h"
#include "FileUtility.h"
#include "Camera.h"
namespace KI
{

GaussianSplattingData* GaussianSplattingLoader::Load(const String& filePath)
{
	std::ifstream file(filePath, std::ios::binary);

	std::string line;
	int vertexNum = 0;
	int floatNum = 0;
	while (std::getline(file, line)) {
		if(StringUtility::Contains(line, "element vertex")) {
			auto contents = StringUtility::Split(line, ' ');
			vertexNum = StringUtility::ToInt(contents[2]);
		} else if (StringUtility::Contains(line, "property float")) {
			floatNum++;
		}else if (StringUtility::Contains(line, "end_header")) {
			break;
		}
	}

	std::vector<float> data(vertexNum * floatNum);

	file.read(reinterpret_cast<char*>(data.data()), sizeof(float) * data.size());
	GaussianSplattingData* splattingData = new GaussianSplattingData(vertexNum);

	int fNum = 0;
	for (size_t i = 0; i < vertexNum; i++) {
		GaussianSplattingData::Vertex vertex{};
		vertex.position = Vector4(data[fNum++], data[fNum++], data[fNum++], 0.0f);
		vertex.normal = Vector4(data[fNum++], data[fNum++], data[fNum++], 0.0f);
		vertex.shCoeff0 = Vector4(data[fNum++], data[fNum++], data[fNum++], 0.0f);
		vertex.shCoeff1 = Vector4(data[fNum++], data[fNum++], data[fNum++], 0.0f);
		vertex.shCoeff2 = Vector4(data[fNum++], data[fNum++], data[fNum++], 0.0f);
		vertex.shCoeff3 = Vector4(data[fNum++], data[fNum++], data[fNum++], 0.0f);
		vertex.shCoeff4 = Vector4(data[fNum++], data[fNum++], data[fNum++], 0.0f);
		vertex.shCoeff5 = Vector4(data[fNum++], data[fNum++], data[fNum++], 0.0f);
		vertex.shCoeff6 = Vector4(data[fNum++], data[fNum++], data[fNum++], 0.0f);
		vertex.shCoeff7 = Vector4(data[fNum++], data[fNum++], data[fNum++], 0.0f);
		vertex.shCoeff8 = Vector4(data[fNum++], data[fNum++], data[fNum++], 0.0f);
		vertex.shCoeff9 = Vector4(data[fNum++], data[fNum++], data[fNum++], 0.0f);
		vertex.shCoeff10 = Vector4(data[fNum++], data[fNum++], data[fNum++], 0.0f);
		vertex.shCoeff11 = Vector4(data[fNum++], data[fNum++], data[fNum++], 0.0f);
		vertex.shCoeff12 = Vector4(data[fNum++], data[fNum++], data[fNum++], 0.0f);
		vertex.shCoeff13 = Vector4(data[fNum++], data[fNum++], data[fNum++], 0.0f);
		vertex.shCoeff14 = Vector4(data[fNum++], data[fNum++], data[fNum++], 0.0f);
		vertex.shCoeff15 = Vector4(data[fNum++], data[fNum++], data[fNum++], 0.0f);
		vertex.scale = Vector3(data[fNum++], data[fNum++], data[fNum++]);
		vertex.opacity = data[fNum++];
		vertex.rotate = Vector4(data[fNum++], data[fNum++], data[fNum++], data[fNum++]);
		splattingData->SetVertex(i, std::move(vertex));
	}


	return splattingData;
}

ShaderPath GaussianSplattingNode::Shader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("common.h");
	path.shader[SHADER_PROGRAM_COMPUTE] = "gaussianSplatting.comp";
	if (depthPhase) {
		path.extension[SHADER_PROGRAM_COMPUTE].push_back("#define WRITE_PHASE\n");
	}
	if (useAccum) {
		path.extension[SHADER_PROGRAM_COMPUTE].push_back("#define DRAW_BY_ACCUM\n");
	}
	return path;
}

void GaussianSplattingNode::Shader::FetchUniformLocation()
{
	m_uImageSize = GetUniformLocation("u_imageSize");
	m_uVP = GetUniformLocation("u_VP");
	m_uVertexNum = GetUniformLocation("u_vertexNum");
	m_uScale = GetUniformLocation("u_scale");
}

void GaussianSplattingNode::Shader::Execute(const DrawContext& context, const GaussianSplattingNode& node, int vertexBuffer, float scale)
{
	auto pColorTarget = context.pResource->GetComputeColorTarget();
	auto pDepthTarget = context.pResource->GetComputeDepthTarget();
	auto pAccumTarget = context.pResource->GetComputeAccumTarget();
	Matrix4x4 vp = context.pResource->GetCamera()->Projection() * context.pResource->GetCamera()->ViewMatrix();
	Use();
	if (depthPhase) {
		BindShaderStorage(1, pDepthTarget->Handle());
	} else {
		BindShaderStorage(0, pColorTarget->Handle());
		BindShaderStorage(1, pDepthTarget->Handle());
	}
	if (useAccum) {
		BindShaderStorage(2, pAccumTarget->Handle());
	}
	BindShaderStorage(3, vertexBuffer);
	BindShaderStorage(4, context.pResource->GetCameraBuffer()->Handle());
	BindUniform(m_uImageSize, context.pResource->GL()->GetWindowSize());
	BindUniform(m_uVP, vp * node.GetMatrix());
	BindUniform(m_uVertexNum, (int)node.GetData()->GetVertexs().size());
	BindUniform(m_uScale, scale * 1000);
	if (IsScreenDispatch()) {
		auto size = context.pResource->GL()->GetWindowSize();
		Dispatch(GetDispatchNum1D(size.x * size.y));
	} else {
		Dispatch(GetDispatchNum1D(node.GetData()->GetVertexs().size()));
	}
	BarrierSSBO();
	UnUse();
}

GaussianSplattingNode::GaussianSplattingNode(const String& name, const Shared<GaussianSplattingData>& data)
	: RenderNode(name)
	, m_data(data)
	, m_pVertexGpu(nullptr)
{

}

void GaussianSplattingNode::BuildGLResource()
{
	if (m_pVertexGpu) { return; }
	m_pVertexGpu = std::make_shared<GLBuffer>();
	m_pVertexGpu->Create(m_data->GetVertexs().size(), sizeof(GaussianSplattingData::Vertex));
	m_pVertexGpu->BufferSubData(0, m_data->GetVertexs());
	m_pShader = std::make_shared<Shader>();
	m_pShader->Build();
	m_pWirteDepthShader = std::make_shared<Shader>();
	m_pWirteDepthShader->SetDepthPhase(true);
	m_pWirteDepthShader->Build();

	m_pAccumShader = std::make_shared<Shader>();
	m_pAccumShader->SetUseAccum(true);
	m_pAccumShader->Build();

	m_pAccumeWriteDepthShader = std::make_shared<Shader>();
	m_pAccumeWriteDepthShader->SetUseAccum(true);
	m_pAccumeWriteDepthShader->SetDepthPhase(true);
	m_pAccumeWriteDepthShader->Build();
}
void GaussianSplattingNode::Draw(const DrawContext& context)
{
	BuildGLResource();

	if (m_ui.useAccum) {
		m_pAccumeWriteDepthShader->Execute(context, *this, m_pVertexGpu->Handle(), m_ui.m_scale);
		m_pAccumShader->Execute(context, *this, m_pVertexGpu->Handle(), m_ui.m_scale);
	} else {
		m_pWirteDepthShader->Execute(context, *this, m_pVertexGpu->Handle(),m_ui.m_scale);
		m_pShader->Execute(context, *this, m_pVertexGpu->Handle(),m_ui.m_scale);
	}

}

void GaussianSplattingNode::ShowUI(UIContext& ui)
{
	ImGui::SliderFloat("Scale", &m_ui.m_scale, 1, 1000);
	ImGui::Checkbox("Use Accum", &m_ui.useAccum);
}
}