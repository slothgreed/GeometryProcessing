#ifndef ISHADER_H
#define ISHADER_H

#define ATTRIB_POSITION 0
#define ATTRIB_NORMAL   1
#define ATTRIB_COLOR   2

#include "GLBuffer.h"
class IShader
{
public:
	IShader();
	virtual ~IShader();

	void Build();
	virtual void Use();
	void Delete();

	virtual std::string GetVertexPath() = 0;
	virtual std::string GetFragmentPath() = 0;
	virtual void GetUniformLocation() = 0;
	virtual void SetViewProj(const mat4x4& value) = 0;
	virtual void SetModel(const mat4x4& value) { assert(0); };

protected:
	GLuint GetId() { return m_programId; };
private:
	GLuint m_programId;
};


#endif ISHADER_H