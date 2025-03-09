#include "IShader.h"
#include "ShaderUtility.h"
#include "Texture.h"
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
	// 色がおかしいのはglVertexAttribBindingのせい。コメントアウトすると正常になる。
	glVertexAttribBinding(format.location, format.binding);
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

void IShadingShader::DrawArray(GLuint primitiveType, GLBuffer* pPositionBuffer)
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

	auto vertexEx = Join(shaderPath.extension[SHADER_PROGRAM_VERTEX]);
	vertexCode = ShaderUtility::LoadFromFile(localPath + shaderPath.shader[SHADER_PROGRAM_VERTEX]);
	vertexCode = version + vertexEx + headerCode + vertexCode;


	if (shaderPath.shader[SHADER_PROGRAM_GEOM].size() != 0) {
		auto geomEx = Join(shaderPath.extension[SHADER_PROGRAM_GEOM]);
		geomCode = ShaderUtility::LoadFromFile(localPath + shaderPath.shader[SHADER_PROGRAM_GEOM]);
		geomCode = version + geomEx + headerCode + geomCode;
	}


	auto fragEx = Join(shaderPath.extension[SHADER_PROGRAM_FRAG]);
	fragCode = ShaderUtility::LoadFromFile(localPath + shaderPath.shader[SHADER_PROGRAM_FRAG]);
	fragCode = version + fragEx + headerCode + fragCode;


	m_programId = IShadingShader::BuildVertexGeomFrag(vertexCode, geomCode, fragCode);


	FetchUniformLocation();
	OUTPUT_GLERROR;
}

GLuint IShadingShader::BuildVertexGeomFrag(const String& vert, const String& geom, const String& frag)
{
	GLuint vertId = 0;
	GLuint geomId = 0;
	GLuint fragId = 0;

	if (vert.size() != 0) { vertId = ShaderUtility::Compile(vert, GL_VERTEX_SHADER); }
	if (geom.size() != 0) { geomId = ShaderUtility::Compile(geom, GL_GEOMETRY_SHADER); }
	if (frag.size() != 0) { fragId = ShaderUtility::Compile(frag, GL_FRAGMENT_SHADER); }
	
	GLuint programId = ShaderUtility::Link(vertId, geomId, fragId);

	if (vertId != 0) { glDeleteShader(vertId); }
	if (geomId != 0) { glDeleteShader(geomId); }
	if (fragId != 0) { glDeleteShader(fragId); }
	return programId;
}
GLuint IShadingShader::BuildVertexFrag(const String& vert, const String& frag)
{
	GLuint vertexId = ShaderUtility::Compile(vert, GL_VERTEX_SHADER);
	GLuint fragId = ShaderUtility::Compile(frag, GL_FRAGMENT_SHADER);
	GLuint programId = ShaderUtility::Link(vertexId, fragId);

	glDeleteShader(vertexId);
	glDeleteShader(fragId);
	return programId;
}


void IShadingShader::BindIndexBuffer(const GLBuffer* pBuffer)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pBuffer->Handle());
	OUTPUT_GLERROR;
}


void IMeshShader::Build()
{
	auto shaderPath = GetShaderPath();
	String localPath = "E:\\MyProgram\\KIProject\\PointCloudApp\\PointCloudApp\\Shader\\";
	auto version = ShaderUtility::LoadFromFile(localPath + shaderPath.version);
	auto headerCode = LoadHeaderCode(localPath, shaderPath.header);

	auto meshEx = Join(shaderPath.extension[SHADER_PROGRAM_MESH]);
	auto fragEx = Join(shaderPath.extension[SHADER_PROGRAM_FRAG]);
	auto meshCode = ShaderUtility::LoadFromFile(localPath + shaderPath.shader[SHADER_PROGRAM_MESH]);
	auto fragCode = ShaderUtility::LoadFromFile(localPath + shaderPath.shader[SHADER_PROGRAM_FRAG]);
	GLuint meshId = ShaderUtility::Compile(version + meshEx + headerCode + meshCode, GL_MESH_SHADER_NV);
	GLuint fragId = ShaderUtility::Compile(version + fragEx + headerCode + fragCode, GL_FRAGMENT_SHADER);

	m_programId = ShaderUtility::Link(meshId, fragId);
	FetchUniformLocation();

	glDeleteShader(meshId);
	glDeleteShader(fragId);
	return;
}
void IComputeShader::Build()
{
	String localPath = "E:\\MyProgram\\KIProject\\PointCloudApp\\PointCloudApp\\Shader\\";
	auto shaderPath = GetShaderPath();
	auto version = ShaderUtility::LoadFromFile(localPath + shaderPath.version);
	auto computeEx = Join(shaderPath.extension[SHADER_PROGRAM_COMPUTE]);
	auto headerCode = LoadHeaderCode(localPath, shaderPath.header);
	auto computeCode = ShaderUtility::LoadFromFile(localPath + shaderPath.shader[SHADER_PROGRAM_COMPUTE]);

	GLuint computeId = ShaderUtility::Compile(version + computeEx + headerCode + computeCode, GL_COMPUTE_SHADER);

	m_programId = ShaderUtility::LinkCompute(computeId);

	FetchUniformLocation();
	OUTPUT_GLERROR;
}

Vector3i IComputeShader::GetDispatchNum1D(const Vector3i& localSize, int value)
{
	return Vector3i((value + localSize.x - 1) / localSize.x, 1, 1);
}

int IShader::GetUniformLocation(const char* str)
{
	auto val = glGetUniformLocation(Handle(), str);
	OUTPUT_GLERROR;
	return val;
}
void IShader::Bind(int location, GLBuffer* pBuffer)
{

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
void IShader::BindUniform(int location, const Vector3& value)
{
	glUniform3f(location, value.x, value.y, value.z);
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

ShaderPath TextureShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("common.h");

	path.shader[SHADER_PROGRAM_VERTEX] = "texture.vert";
	if (m_type == Type::VEC4) {
		path.shader[SHADER_PROGRAM_FRAG] = "texture.frag";
	} else if(m_type == Type::UINT) {
		path.shader[SHADER_PROGRAM_FRAG] = "textureUINT.frag";
	}

	return path;
}

void IMeshShader::DrawMeshTasks(int first, int count)
{
	glDrawMeshTasksNV(first, count);
	OUTPUT_GLERROR;
}
void TextureShader::FetchUniformLocation()
{
	m_uniform = GetUniformLocation("tex");
}

void TextureShader::BindTexture(const Texture& texture)
{
	glActiveTexture(GL_TEXTURE0);
	OUTPUT_GLERROR;
	glUniform1i(m_uniform, 0);
	OUTPUT_GLERROR;
	glBindTexture(GL_TEXTURE_2D, texture.Handle());
	OUTPUT_GLERROR;
}

void TextureShader::SetPosition(GLBuffer* pPosition)
{
	static const int ATTRIBUTE_POSITION = 0;
	SetVertexFormat(VertexFormat(ATTRIBUTE_POSITION, pPosition));

	glBindVertexBuffer(ATTRIBUTE_POSITION, pPosition->Handle(), 0, pPosition->SizeOfData());
	OUTPUT_GLERROR;
}

void TextureShader::SetTexture(GLBuffer* pTexture)
{
	static const int ATTRIBUTE_TEXCOORD = 1;
	SetVertexFormat(VertexFormat(ATTRIBUTE_TEXCOORD, pTexture));

	glBindVertexBuffer(ATTRIBUTE_TEXCOORD, pTexture->Handle(), 0, pTexture->SizeOfData());
	OUTPUT_GLERROR;

}


}