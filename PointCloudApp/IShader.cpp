#include "IShader.h"
#include "ShaderUtility.h"
#include "Texture.h"
#include "Primitives.h"
#include "PostEffect.h"
namespace KI
{
IShader::~IShader()
{
	Delete();
}
void IShader::Use()
{
	assert(m_programId != 0);
	glUseProgram(m_programId);
	OUTPUT_GLERROR;
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	OUTPUT_GLERROR;
}

void IShader::UnUse()
{
	glUseProgram(0);
	OUTPUT_GLERROR;
}

void IShader::Delete()
{
	if (m_programId == 0) {
		return;
	}
	glDeleteProgram(m_programId);
	m_programId = 0;
}


String IShader::LoadHeaderCode(const String& localPath, const Vector<String>& header)
{
	String headerCode;
	for (size_t i = 0; i < header.size(); i++) {
		headerCode += ShaderUtility::LoadFromFile(localPath + header[i]);
	}

	return headerCode;
}

void IShadingShader::SetVertexFormat(const VertexFormats& formats)
{
	for (const auto& format : formats) {
		SetVertexFormat(format);
	}
}

void IShadingShader::SetVertexFormat(const VertexFormat& format)
{
	glEnableVertexAttribArray(format.location);
	glVertexAttribFormat(format.location, format.componentSize, format.type, format.normalized, format.offset);
	glVertexAttribBinding(format.location, format.binding);
}

void IShadingShader::PatchParameteri(int num)
{
	glPatchParameteri(GL_PATCH_VERTICES, num);
}
void IShadingShader::DrawElement(GLuint primitiveType, GLBuffer* pIndexBuffer)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pIndexBuffer->Handle());
	glDrawElements(primitiveType, pIndexBuffer->Num(), pIndexBuffer->DataType(), 0);
	OUTPUT_GLERROR;
}

void IShadingShader::DrawElement(GLuint primitiveType, GLBuffer* pIndexBuffer, int num, int offset)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pIndexBuffer->Handle());
	offset *= sizeof(unsigned int);
	glDrawElements(primitiveType, num, pIndexBuffer->DataType(), (void*)offset);
	OUTPUT_GLERROR;
}


void IShadingShader::DrawElementInstaced(GLuint primitiveType, GLBuffer* pIndexBuffer, int instaceNum)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pIndexBuffer->Handle());
	glDrawElementsInstanced(primitiveType, pIndexBuffer->Num(), pIndexBuffer->DataType(), 0, instaceNum);
	OUTPUT_GLERROR;
}

void IShadingShader::DrawArrayInstaced(GLuint primitiveType, int count, int instaceNum)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDrawArraysInstanced(primitiveType, 0, count, instaceNum);
	OUTPUT_GLERROR;
}

void IShadingShader::DrawArray(GLuint primitiveType, const GLBuffer* pPositionBuffer)
{
	DrawArray(primitiveType, pPositionBuffer->Num());
}

void IShadingShader::DrawArray(GLuint primitiveType, int count)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDrawArrays(primitiveType, 0, count);
	OUTPUT_GLERROR;
}

void IShadingShader::DrawArray(GLuint primitiveType, int offset, int count)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDrawArrays(primitiveType, offset, count);
	OUTPUT_GLERROR;
}

void IShadingShader::DrawElementsBaseVertex(const DrawArgs& args)
{
	glDrawElementsBaseVertex(args.primitive, args.count, args.dataType, (void*)args.offset, args.baseVertex);
	OUTPUT_GLERROR;

}
void IShadingShader::DrawElementsBaseVertex(GLuint primitiveType, uint count, GLuint type, void* offset, uint baseVertex)
{
	glDrawElementsBaseVertex(primitiveType, count, GL_UNSIGNED_INT, (void*)offset, baseVertex);
	OUTPUT_GLERROR;
}


IShadingShader::IShadingShader()
{

}

IShadingShader::~IShadingShader()
{

}

String Join(const Vector<String>& strs)
{
	String ret;
	for (const auto& extension : strs) {
		ret += extension;
	}

	return ret;
}
void IShadingShader::Build()
{
	auto shaderPath = GetShaderPath();
	String localPath = "E:\\MyProgram\\KIProject\\PointCloudApp\\PointCloudApp\\Shader\\";
	auto version = ShaderUtility::LoadFromFile(localPath + shaderPath.version);
	auto headerCode = LoadHeaderCode(localPath, shaderPath.header);
	String vertexCode;
	String fragCode;
	String geomCode;
	String tesCCode;
	String tesECode;

	auto vertexEx = Join(shaderPath.extension[SHADER_PROGRAM_VERTEX]);
	vertexCode = ShaderUtility::LoadFromFile(localPath + shaderPath.shader[SHADER_PROGRAM_VERTEX]);
	vertexCode = version + vertexEx + headerCode + vertexCode;


	if (shaderPath.shader[SHADER_PROGRAM_GEOM].size() != 0) {
		auto geomEx = Join(shaderPath.extension[SHADER_PROGRAM_GEOM]);
		geomCode = ShaderUtility::LoadFromFile(localPath + shaderPath.shader[SHADER_PROGRAM_GEOM]);
		geomCode = version + geomEx + headerCode + geomCode;
	}

	if (shaderPath.shader[SHADER_PROGRAM_TES_CONTROL].size() != 0) {
		auto tesEx = Join(shaderPath.extension[SHADER_PROGRAM_TES_CONTROL]);
		tesCCode = ShaderUtility::LoadFromFile(localPath + shaderPath.shader[SHADER_PROGRAM_TES_CONTROL]);
		tesCCode = version + tesEx + headerCode + tesCCode;
	}

	if (shaderPath.shader[SHADER_PROGRAM_TES_EVAL].size() != 0) {
		auto tesEx = Join(shaderPath.extension[SHADER_PROGRAM_TES_EVAL]);
		tesECode = ShaderUtility::LoadFromFile(localPath + shaderPath.shader[SHADER_PROGRAM_TES_EVAL]);
		tesECode = version + tesEx + headerCode + tesECode;
	}

	auto fragEx = Join(shaderPath.extension[SHADER_PROGRAM_FRAG]);
	fragCode = ShaderUtility::LoadFromFile(localPath + shaderPath.shader[SHADER_PROGRAM_FRAG]);
	fragCode = version + fragEx + headerCode + fragCode;


	m_programId = IShadingShader::Compile(vertexCode, tesCCode, tesECode, geomCode, fragCode);


	FetchUniformLocation();
	OUTPUT_GLERROR;
}

GLuint IShadingShader::Compile(const String& vert, const String& tesc, const String& tese, const String& geom, const String& frag)
{
	GLuint vertId = 0;
	GLuint tescId = 0;
	GLuint teseId = 0;
	GLuint geomId = 0;
	GLuint fragId = 0;

	if (vert.size() != 0) { vertId = ShaderUtility::Compile(vert, GL_VERTEX_SHADER); }
	if (tesc.size() != 0) { tescId = ShaderUtility::Compile(tesc, GL_TESS_CONTROL_SHADER); }
	if (tese.size() != 0) { teseId = ShaderUtility::Compile(tese, GL_TESS_EVALUATION_SHADER); }
	if (geom.size() != 0) { geomId = ShaderUtility::Compile(geom, GL_GEOMETRY_SHADER); }
	if (frag.size() != 0) { fragId = ShaderUtility::Compile(frag, GL_FRAGMENT_SHADER); }
	
	GLuint programId = ShaderUtility::Link(vertId, tescId, teseId, geomId, fragId);

	if (vertId != 0) { glDeleteShader(vertId); }
	if (tescId != 0) { glDeleteShader(tescId); }
	if (teseId != 0) { glDeleteShader(teseId); }
	if (geomId != 0) { glDeleteShader(geomId); }
	if (fragId != 0) { glDeleteShader(fragId); }
	return programId;
}


void IShadingShader::BindTexture(int location, int unit, const Texture& texture)
{
	glActiveTexture(GL_TEXTURE0 + unit);
	OUTPUT_GLERROR;
	glUniform1i(location, unit);
	OUTPUT_GLERROR;
	glBindTexture(texture.GetFormat().target, texture.Handle());
	OUTPUT_GLERROR;
}


void IShadingShader::BindCubemap(int location, int unit, const CubemapTexture& texture)
{
	glActiveTexture(GL_TEXTURE0 + unit);
	OUTPUT_GLERROR;
	glUniform1i(location, unit);
	OUTPUT_GLERROR;
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture.Handle());
	OUTPUT_GLERROR;
}
void IShadingShader::BindIndexBuffer(const GLBuffer* pBuffer)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pBuffer->Handle());
	OUTPUT_GLERROR;
}

void IShadingShader::DrawIndirectBuffer(const GLBuffer* pBuffer)
{
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, pBuffer->Handle());
	OUTPUT_GLERROR;
}
void IShadingShader::DrawArrayIndirect(GLuint primitiveType, int offset)
{
	glDrawArraysIndirect(primitiveType, (void*)offset);
	OUTPUT_GLERROR;
}

int IMeshShader::GetMaxVertices() const
{
	return GLAPIExt::Info()->GetMeshletMaxVertex();
}
int IMeshShader::GetMaxPrimitives() const
{
	return GLAPIExt::Info()->GetMeshletMaxPrimitive();
}

int IMeshShader::GetDispatchNum(int num)
{
	return (num + GetTaskThreadNum() - 1) / GetTaskThreadNum();
}

void IMeshShader::BarrierSSBO()
{
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	OUTPUT_GLERROR;
}
void IMeshShader::Draw(int first, int count)
{
	glDrawMeshTasksNV(first, count);
	OUTPUT_GLERROR;
}

void IMeshShader::DrawWithAutoTask(int first, int count)
{
	Draw(first, GetDispatchNum(count));
}
void IMeshShader::Build()
{
	auto shaderPath = GetShaderPath();
	shaderPath.extension[SHADER_PROGRAM_TASK].push_back("#extension GL_NV_mesh_shader : require\n");
	shaderPath.extension[SHADER_PROGRAM_MESH].push_back("#extension GL_NV_mesh_shader : require\n");
	String localPath = "E:\\MyProgram\\KIProject\\PointCloudApp\\PointCloudApp\\Shader\\";
	auto version = ShaderUtility::LoadFromFile(localPath + shaderPath.version);
	auto headerCode = LoadHeaderCode(localPath, shaderPath.header);

	auto meshEx = Join(shaderPath.extension[SHADER_PROGRAM_MESH]);
	auto taskEx = Join(shaderPath.extension[SHADER_PROGRAM_TASK]);
	auto fragEx = Join(shaderPath.extension[SHADER_PROGRAM_FRAG]);

	auto taskLocalSize = String("layout(local_size_x=" + IntToString(GetTaskThreadNum()) + ") in;\n");
	auto meshLocalSize = String("layout(local_size_x="+ IntToString(GetMeshThreadNum()) + ") in;\n");
	auto maxVertex = GetMaxVertices();
	if (maxVertex > GLAPIExt::Info()->GetMeshletMaxVertex()) {
		assert(0);
		maxVertex = GLAPIExt::Info()->GetMeshletMaxVertex();
	}
	auto maxPrimitive = GetMaxPrimitives();
	if (maxPrimitive > GLAPIExt::Info()->GetMeshletMaxPrimitive()) {
		assert(0);
		maxPrimitive = GLAPIExt::Info()->GetMeshletMaxPrimitive();
	}
	auto meshOutSize = String("layout(max_vertices="+IntToString(maxVertex) + ", max_primitives="+IntToString(maxPrimitive) + ") out;\n");
	auto meshCode = ShaderUtility::LoadFromFile(localPath + shaderPath.shader[SHADER_PROGRAM_MESH]);
	auto taskCode = ShaderUtility::LoadFromFile(localPath + shaderPath.shader[SHADER_PROGRAM_TASK]);
	auto fragCode = ShaderUtility::LoadFromFile(localPath + shaderPath.shader[SHADER_PROGRAM_FRAG]);
	GLuint meshId = ShaderUtility::Compile(version + meshEx + headerCode + meshLocalSize + meshOutSize + meshCode, GL_MESH_SHADER_NV);
	GLuint fragId = ShaderUtility::Compile(version + fragEx + headerCode + fragCode, GL_FRAGMENT_SHADER);
	GLuint taskId = 0;
	if (!taskCode.empty()) {
		taskId = ShaderUtility::Compile(version + taskEx + headerCode + taskLocalSize + taskCode, GL_TASK_SHADER_NV);
	}
	m_programId = ShaderUtility::Link(meshId, taskId, fragId);
	FetchUniformLocation();

	if (meshId != 0) glDeleteShader(meshId); meshId = 0;
	if (taskId != 0) glDeleteShader(taskId); taskId = 0;
	if (fragId != 0) glDeleteShader(fragId); fragId = 0;
	return;
}

int IShader::GetUniformLocation(const char* str)
{
	auto val = glGetUniformLocation(Handle(), str);
	OUTPUT_GLERROR;
	return val;
}
void IShader::Bind(int location, const GLBuffer* pBuffer)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, location, pBuffer->Handle());
	OUTPUT_GLERROR;
}

void IShader::BindUniform(int location, const Matrix4x4& value)
{
	glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
	OUTPUT_GLERROR;
}
void IShader::BindUniform(int location, const Vector2i& value)
{
	glUniform2i(location, value.x, value.y);
	OUTPUT_GLERROR;
}
void IShader::BindUniform(int location, const Vector2& value)
{
	glUniform2f(location, value.x, value.y);
	OUTPUT_GLERROR;
}

void IShader::BindUniform(int location, const Vector3& value)
{
	glUniform3f(location, value.x, value.y, value.z);
	OUTPUT_GLERROR;
}

void IShader::BindUniform(int location, const Vector4& value)
{
	glUniform4f(location, value.x, value.y, value.z, value.w);
	OUTPUT_GLERROR;
}

void IShader::BindUniform(int location, const Vector3i& value)
{
	glUniform3i(location, value.x, value.y, value.z);
	OUTPUT_GLERROR;
}

void IShader::BindUniform(int location, float value)
{
	glUniform1f(location, value);
	OUTPUT_GLERROR;
}
void IShader::BindUniform(int location, int value)
{
	glUniform1i(location, value);
	OUTPUT_GLERROR;
}

void IShader::BindUniform(int location, uint64 value)
{
	glUniform1ui64ARB(location, value);
	OUTPUT_GLERROR;
}

void IShader::BindUniform(int location, uint value)
{
	glUniform1ui(location, value);
	OUTPUT_GLERROR;
}
void IShader::BindShaderStorage(int location, int handle)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, location, handle);
	OUTPUT_GLERROR;
}

void IComputeShader::Build()
{
	String localPath = "E:\\MyProgram\\KIProject\\PointCloudApp\\PointCloudApp\\Shader\\";
	auto shaderPath = GetShaderPath();
	auto version = ShaderUtility::LoadFromFile(localPath + shaderPath.version);
	auto computeEx = Join(shaderPath.extension[SHADER_PROGRAM_COMPUTE]);
	auto headerCode = LoadHeaderCode(localPath, shaderPath.header);
	auto localSize = GetLocalThreadNum();
	GLAPIExt::Info()->GetMaxComputeWorkGroupCount();
	auto maxLocalSize = GLAPIExt::Info()->GetMaxComputeLocalSize();
	if (localSize.x > maxLocalSize.x ||
		localSize.y > maxLocalSize.y ||
		localSize.z > maxLocalSize.z ||
		localSize.x * localSize.y * localSize.z > maxLocalSize.w) {
		assert(0);
	}
	auto threadStr = "layout(local_size_x = " + IntToString(localSize.x) + ", local_size_y = " + IntToString(localSize.y) + ", local_size_z = " + IntToString(localSize.z) + ") in;\n";
	auto computeCode = ShaderUtility::LoadFromFile(localPath + shaderPath.shader[SHADER_PROGRAM_COMPUTE]);

	GLuint computeId = ShaderUtility::Compile(version + computeEx + threadStr + headerCode + computeCode, GL_COMPUTE_SHADER);

	m_programId = ShaderUtility::LinkCompute(computeId);

	FetchUniformLocation();
	OUTPUT_GLERROR;
}

Vector3i IComputeShader::GetLocalThreadNum() const
{
	return Vector3i(1, 1, 1);
}

Vector3i IComputeShader::GetDispatchNum2D(const Vector2i& value)
{
	auto localSize = GetLocalThreadNum();
	return Vector3i((value.x + localSize.x - 1) / localSize.x, (value.y + localSize.y - 1) / localSize.y, 1);
}
Vector3i IComputeShader::GetDispatchNum1D(int value)
{
	auto localSize = GetLocalThreadNum();
	return Vector3i((value + localSize.x - 1) / localSize.x, 1, 1);
}

void IComputeShader::BindTexture(int location, int mipmap, const Texture* pTexture, GLuint access)
{
	if (!(access == GL_WRITE_ONLY ||
		access == GL_READ_ONLY ||
		access == GL_READ_WRITE)) {
		assert(0);
		return;
	}

	GLboolean layerd = GL_FALSE;
	if (pTexture->GetFormat().target == GL_TEXTURE_CUBE_MAP_ARRAY ||
		pTexture->GetFormat().target == GL_TEXTURE_2D_ARRAY) {
		layerd = GL_TRUE;
	}

	OUTPUT_GLERROR;
	glBindImageTexture(location, pTexture->Handle(), mipmap, layerd, 0, access, pTexture->GetFormat().internalformat);
	OUTPUT_GLERROR;
}
void IComputeShader::BindTexture(int location, const Texture* pTexture, GLuint access)
{
	BindTexture(location, 0, pTexture, access);
}

void IComputeShader::BarrierSSBO()
{
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	OUTPUT_GLERROR;
}

void IComputeShader::BarrierSSBOAndCommand()
{
	glMemoryBarrier(
		GL_SHADER_STORAGE_BARRIER_BIT |
		GL_COMMAND_BARRIER_BIT);
	OUTPUT_GLERROR;
}

void IComputeShader::Dispatch1D(GLuint x)
{
	glDispatchCompute(x, 1, 1);
	OUTPUT_GLERROR;
}
void IComputeShader::Dispatch(GLuint x, GLuint y, GLuint z)
{
	glDispatchCompute(x, y, z);
	OUTPUT_GLERROR;
}

void IComputeShader::Dispatch(const Vector3i& value)
{
	glDispatchCompute(value.x, value.y, value.z);
	OUTPUT_GLERROR;
}

void IComputeShader::BarrierImage()
{
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	OUTPUT_GLERROR;
}



void IPostEffectShader::Draw(const RenderTextureNode& node)
{
	SetPosition(node.GetPositionBuffer().get());
	SetTexcoord(node.GetTexcoordBuffer().get());
	DrawElement(RenderPlane::GetPrimitiveType(), node.GetIndexBuffer().get());
}


void IPostEffectShader::SetPosition(GLBuffer* pPosition)
{
	static const int ATTRIBUTE_POSITION = 0;
	SetVertexFormat(VertexFormat(ATTRIBUTE_POSITION, pPosition));

	glBindVertexBuffer(ATTRIBUTE_POSITION, pPosition->Handle(), 0, pPosition->SizeOfData());
	OUTPUT_GLERROR;
}


void IPostEffectShader::SetTexcoord(GLBuffer* pTexture)
{
	static const int ATTRIBUTE_TEXCOORD = 1;
	SetVertexFormat(VertexFormat(ATTRIBUTE_TEXCOORD, pTexture));

	glBindVertexBuffer(ATTRIBUTE_TEXCOORD, pTexture->Handle(), 0, pTexture->SizeOfData());
	OUTPUT_GLERROR;
}



}