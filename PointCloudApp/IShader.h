#ifndef ISHADER_H
#define ISHADER_H

#define ATTRIB_POSITION 0
#define ATTRIB_NORMAL   1
#define ATTRIB_COLOR   2
#define ATTRIB_TEXCOORD   2

#include "GLBuffer.h"
namespace KI
{
class RenderTarget;
class Texture;
class CubemapTexture;
enum SHADER_PROGRAM
{
	SHADER_PROGRAM_VERTEX,
	SHADER_PROGRAM_GEOM,
	SHADER_PROGRAM_FRAG,
	SHADER_PROGRAM_TASK,
	SHADER_PROGRAM_MESH,
	SHADER_PROGRAM_COMPUTE
};

struct ShaderPath
{
	String version;
	Vector<String> header;
	std::unordered_map<SHADER_PROGRAM, Vector<String>> extension;
	std::unordered_map<SHADER_PROGRAM, String> shader;
};


class IShader
{
public:
	IShader()
		: m_programId(0) {};
	~IShader();

	virtual void Build() = 0;
	virtual ShaderPath GetShaderPath() = 0;
	bool IsActive() { return m_programId != 0; }
	virtual void Use();
	void UnUse();
	void Delete();
	int GetUniformLocation(const char* str);
	void Bind(int location, GLBuffer* pBuffer);
	void BindUniform(int location, const Matrix4x4& value);
	void BindUniform(int location, const Vector2& value);
	void BindUniform(int location, const Vector2i& value);
	void BindUniform(int location, const Vector3i& value);
	void BindUniform(int location, const Vector3& value);
	void BindUniform(int location, float value);
	void BindUniform(int location, int value);
	void BindUniform(int location, uint64 value);
	void BindUniform(int location, uint value);
	void BindShaderStorage(int location, int handle);
protected:
	String LoadHeaderCode(const String& localPath, const Vector<String>& header);
	GLuint Handle() const { return m_programId; };
	GLuint m_programId;
};


class IShadingShader : public IShader
{
public:

	IShadingShader();
	virtual ~IShadingShader();
	virtual void Build();
	virtual void FetchUniformLocation() = 0;
	virtual void SetModel(const Matrix4x4& value) { assert(0); };
	virtual int GetDrawTargetNum() const { return 1; }
	void BindTexture(int location, int unit, const Texture& texture);
	void BindCubemap(int location, int unit, const CubemapTexture& texture);
	void BindIndexBuffer(const GLBuffer* pBuffer);
	void SetVertexFormat(const VertexFormats& format);
	void SetVertexFormat(const VertexFormat& format);
	void DrawElement(GLuint primitiveType, GLBuffer* pIndexBuffer);
	void DrawElement(GLuint primitiveType, GLBuffer* pIndexBuffer, int num, int offset);
	void DrawElementInstaced(GLuint primitiveType, GLBuffer* pIndexBuffer, int instanceNum);
	void DrawArrayInstaced(GLuint primitiveType, int count, int instanceNum);
	void DrawArray(GLuint primitiveType, GLBuffer* pIndexBuffer);
	void DrawArray(GLuint primitiveType, int count);
	void DrawArray(GLuint primitiveType, int offset, int count);
	void DrawElementsBaseVertex(const DrawArgs& args);
	void DrawElementsBaseVertex(GLuint primitiveType, uint count, GLuint type, void* offset, uint baseVertex);
private:
	GLuint BuildVertexFrag(const String& vert, const String& frag);
	GLuint BuildVertexGeomFrag(const String& vert, const String& geom, const String& frag);

};

class IComputeShader : public IShader
{
public:
	IComputeShader() {};
	virtual ~IComputeShader() {};

	virtual void FetchUniformLocation() = 0;

	virtual Vector3i GetLocalThreadNum() const;
	Vector3i GetDispatchNum2D(const Vector2i& value);
	Vector3i GetDispatchNum1D(int value);
	virtual void Build();
	void Dispatch(GLuint x, GLuint y, GLuint z);
	void Dispatch(const Vector3i& value);
	void BindTexture(int location, const Texture* pTexture, GLuint access);
	void BindTexture(int location, int mipmap, const Texture* pTexture, GLuint access);
	void BarrierImage();
	void BarrierSSBO();

private:

};

class RenderTextureNode;
class IPostEffectShader : public IShadingShader
{
public:
	IPostEffectShader() {};
	virtual ~IPostEffectShader() {};

	void SetPosition(GLBuffer* pPosition);
	void SetTexcoord(GLBuffer* pTexture);
	void Draw(const RenderTextureNode& node);
};

class IMeshShader : public IShader
{
public:
	IMeshShader() {};
	virtual ~IMeshShader() {};

	virtual int GetTaskThreadNum() const { return 1; }
	virtual int GetMeshThreadNum() const { return 1; }
	virtual int GetMaxVertices() const;
	virtual int GetMaxPrimitives() const;
	int GetDispatchNum(int num);
	void BarrierSSBO();
	virtual void FetchUniformLocation() {};
	virtual void Build();

	void Draw(int first, int count);
	void DrawWithAutoTask(int first, int count);
private:

};


}


#endif ISHADER_H