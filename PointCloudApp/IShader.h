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
	SHADER_PROGRAM_TES_CONTROL,
	SHADER_PROGRAM_TES_EVAL,
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
	static void Bind(int location, const GLBuffer* pBuffer);
	static void BindUniform(int location, const Matrix4x4& value);
	static void BindUniform(int location, const Vector2& value);
	static void BindUniform(int location, const Vector2i& value);
	static void BindUniform(int location, const Vector3i& value);
	static void BindUniform(int location, const Vector3& value);
	static void BindUniform(int location, const Vector4& value);
	static void BindUniform(int location, float value);
	static void BindUniform(int location, int value);
	static void BindUniform(int location, uint64 value);
	static void BindUniform(int location, uint value);
	static void BindShaderStorage(int location, int handle);
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

	// tessellation
	void PatchParameteri(int num);

	void DrawElement(GLuint primitiveType, GLBuffer* pIndexBuffer);
	void DrawElement(GLuint primitiveType, GLBuffer* pIndexBuffer, int num, int offset);
	void DrawElementInstaced(GLuint primitiveType, GLBuffer* pIndexBuffer, int instanceNum);
	void DrawArrayInstaced(GLuint primitiveType, int count, int instanceNum);
	void DrawArray(GLuint primitiveType, const GLBuffer* pIndexBuffer);
	void DrawArray(GLuint primitiveType, int count);
	void DrawArray(GLuint primitiveType, int offset, int count);
	void DrawElementsBaseVertex(const DrawArgs& args);
	void DrawElementsBaseVertex(GLuint primitiveType, uint count, GLuint type, void* offset, uint baseVertex);
private:
	GLuint Compile(const String& vert, const String& tesc, const String& tese, const String & geom, const String& frag);

};

class IComputeShader : public IShader
{
public:
	IComputeShader() {};
	virtual ~IComputeShader() {};

	virtual void FetchUniformLocation() {};

	virtual Vector3i GetLocalThreadNum() const;
	Vector3i GetDispatchNum2D(const Vector2i& value);
	Vector3i GetDispatchNum1D(int value);
	virtual void Build();
	static void Dispatch1D(GLuint x);
	static void Dispatch(GLuint x, GLuint y, GLuint z);
	static void Dispatch(const Vector3i& value);
	static void BindTexture(int location, const Texture* pTexture, GLuint access);
	static void BindTexture(int location, int mipmap, const Texture* pTexture, GLuint access);
	static void BarrierImage();
	static void BarrierSSBO();

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