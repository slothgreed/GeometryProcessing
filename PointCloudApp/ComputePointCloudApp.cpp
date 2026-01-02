#include <iostream>
#include "PointCloudIO.h"
#include "ComputePointCloudApp.h"
#include "PostEffect.h"
#include "Texture.h"
#include "PointCloud.h"
#include "MouseInput.h"
#include "Profiler.h"
#include "FileUtility.h"
namespace KI
{

ShaderPath CreateTextureComputeShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.shader[SHADER_PROGRAM_COMPUTE] = "sample\\texture.comp";
	return path;
}
void CreateTextureComputeShader::Execute(const Shared<Texture>& pTexture)
{
	pTexture->Bind();
	Use();
	OUTPUT_GLERROR;
	glBindImageTexture(0, pTexture->Handle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	OUTPUT_GLERROR;
	glDispatchCompute(pTexture->Size().x, pTexture->Size().y, 1);
	OUTPUT_GLERROR;
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	OUTPUT_GLERROR;
	UnUse();
}

void CreateTextureComputeShader::FetchUniformLocation()
{

}


PointCloudComputeShader::PointCloudComputeShader(const Shared<PointCloud>& pPointCloud)
	:m_pPointCloud(pPointCloud)
{
	Initialize();
}

PointCloudComputeShader::~PointCloudComputeShader()
{
}

ShaderPath PointCloudComputeShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.shader[SHADER_PROGRAM_COMPUTE] = "pointcloud.comp";

	return path;
}

void PointCloudComputeShader::FetchUniformLocation()
{
	m_uniformVP = GetUniformLocation("u_VP");
}
void PointCloudComputeShader::Initialize()
{
	m_pPointBuffer = std::make_unique<GLBuffer>();
	m_pPointBuffer->Create(m_pPointCloud->CreatePositionColor4f());
}

void PointCloudComputeShader::Execute(const Matrix4x4& proj, const Matrix4x4& view, const Shared<Texture2D>& pColorBuffer, const Shared<Texture2D>& pDepthBuffer)
{
	pColorBuffer->Clear(0);
	if (pDepthBuffer) {
		pDepthBuffer->Clear(1);
	}

	Use();
	glBindImageTexture(0, pColorBuffer->Handle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	if (pDepthBuffer) {
		glBindImageTexture(1, pDepthBuffer->Handle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	}

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_pPointBuffer->Handle());

	Matrix4x4 vp = proj * view;
	glUniformMatrix4fv(m_uniformVP, 1, GL_FALSE, &vp[0][0]);
	glDispatchCompute(m_pPointCloud->Position().size() / 1024, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	OUTPUT_GLERROR;
	UnUse();
}


void ComputePointCloudApp::ProcessMouseEvent(const MouseInput& input)
{
	m_pMouse->ApplyMouseInput(input);
	EditContext context(m_pMouse.get(), m_pCamera.get());
	if (input.Event() == MOUSE_EVENT_WHEEL) {
		m_pCameraController->Wheel(context);
	} else if (input.Event() == MOUSE_EVENT_MOVE) {
		m_pCameraController->Move(context);
	}
}

void ComputePointCloudApp::ResizeEvent(int width, int height)
{
	if (m_pColorTexture) {
		m_pColorTexture->Resize(width, height);
	}
	glViewport(0, 0, width, height);
}






#define WIDTH 800
#define HEIGHT 600

// ---------- シェーダ ----------
const char* vertSrc = R"(
#version 450 core
#extension GL_OVR_multiview : require

layout(num_views = 2) in;

layout(location = 0) in vec3 aPos;
out vec3 vColor;

uniform mat4 uView[2];
uniform mat4 uProj[2];

void main() {
    gl_Position = uProj[gl_ViewID_OVR] * uView[gl_ViewID_OVR] * vec4(aPos, 1.0);
    // 左右の目で色を少し変化させる
    vColor = (gl_ViewID_OVR == 0) ? vec3(1,0,0) : vec3(0,1,1);
}
)";

const char* fragSrc = R"(
#version 450 core
in vec3 vColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(vColor, 1.0);
}
)";


// ---------- シェーダユーティリティ ----------
GLuint compileShader(GLenum type, const char* src)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);
	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char log[512];
		glGetShaderInfoLog(shader, 512, nullptr, log);
		std::cerr << "Shader compile error:\n" << log << std::endl;
	}
	return shader;
}

GLuint createProgram()
{
	GLuint vs = compileShader(GL_VERTEX_SHADER, vertSrc);
	GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragSrc);
	GLuint prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);
	GLint success;
	glGetProgramiv(prog, GL_LINK_STATUS, &success);
	if (!success) {
		char log[512];
		glGetProgramInfoLog(prog, 512, nullptr, log);
		std::cerr << "Program link error:\n" << log << std::endl;
	}
	glDeleteShader(vs);
	glDeleteShader(fs);
	return prog;
}
#define WIDTH 800
#define HEIGHT 600
void ComputePointCloudApp::Execute()
{
	/*
	m_pColorTexture = std::make_shared<Texture2D>();
	m_pColorTexture->Build(512, 512);

	//CreateTextureComputeShader shader;
	//shader.Build();
	//shader.Execute(pTexture);

	//auto pPointCloud = (Shared<PointCloud>(PointCloudIO::Load("E:\\MyProgram\\KIProject\\PointCloudApp\\resource\\PointCloud\\cube.xyz")));
	//auto pPointCloud = Shared<PointCloud>(PointCloudIO::Create2D(1000, vec2(-100, -100), vec2(100, 100)));
	//auto pPointCloud = Shared<PointCloud>(PointCloudIO::Create2D(100, vec2(-100, -100), vec2(100, 100)));
	auto pPointCloud = (Shared<PointCloud>(PointCloudIO::Load("E:\\cgModel\\pointCloud\\bildstein_station3_xyz_intensity_rgb.xyz")));
	BDB bdb;
	bdb.Apply(pPointCloud->GetBDB());
	m_pCameraController->FitToBDB(bdb);

	int maxCountX, maxCountY, maxCountZ;
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxCountX);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &maxCountY);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &maxCountZ);

	int maxSizeX, maxSizeY, maxSizeZ;
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &maxSizeX);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &maxSizeY);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &maxSizeZ);

	DrawContext context;
	auto pShader = std::make_unique<PointCloudComputeShader>(pPointCloud);
	pShader->Build();
	auto m_pResource = std::make_unique<RenderResource>();
	m_pResource->Build();

	GPUProfiler profiler("Render");
	auto pNode = std::make_unique<RenderTextureNode>();
	context.pResource = m_pResource.get();
	context.pResource->SetTexturePlane(pNode.get());

	while (glfwWindowShouldClose(m_window) == GL_FALSE) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		profiler.Start();
		pShader->Execute(m_pCamera->Projection(), m_pCamera->ViewMatrix(), m_pColorTexture, nullptr);
		TextureDrawer::Execute(context, m_pColorTexture.get());
		profiler.Stop();

		glfwSwapBuffers(m_window);

		glfwWaitEvents();
		OUTPUT_GLERROR;
	}
	*/










	// 拡張チェック
	const char* ext = (const char*)glGetString(GL_EXTENSIONS);
	auto exts = StringUtility::Split(ext, ' ');
	for (auto ext : exts) {
		if (strstr(ext.data(), "GL_OVR_multiview")) {
			std::cerr << "GL_EXT_multiview support!" << std::endl;
		}
	}
	// 三角形データ
	float vertices[] = {
		-0.6f, -0.5f, 0.0f,
		 0.6f, -0.5f, 0.0f,
		 0.0f,  0.5f, 0.0f
	};

	GLuint vao, vbo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	// プログラム
	GLuint prog = createProgram();
	glUseProgram(prog);

	// FBOセットアップ（左右目をレイヤーで分ける）
	GLuint fbo, colorTex, depthTex;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &colorTex);
	glBindTexture(GL_TEXTURE_2D_ARRAY, colorTex);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, WIDTH, HEIGHT, 2);
	glFramebufferTextureMultiviewOVR(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, colorTex, 0, 0, 2);

	glGenTextures(1, &depthTex);
	glBindTexture(GL_TEXTURE_2D_ARRAY, depthTex);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT, 2);
	glFramebufferTextureMultiviewOVR(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTex, 0, 0, 2);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "FBO incomplete!" << std::endl;
		return;
	}

	// 行列（左右目のわずかな視差）
	GLfloat eyeOffset = 0.05f;
	GLfloat view[2][16] = {
		{1,0,0, eyeOffset, 0,1,0,0, 0,0,1,0, 0,0,0,1},
		{1,0,0,-eyeOffset, 0,1,0,0, 0,0,1,0, 0,0,0,1}
	};
	GLfloat proj[2][16] = {
		{1,0,0,0, 0,1,0,0, 0,0,-1,0, 0,0,0,1},
		{1,0,0,0, 0,1,0,0, 0,0,-1,0, 0,0,0,1}
	};

	glUniformMatrix4fv(glGetUniformLocation(prog, "uView"), 2, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(prog, "uProj"), 2, GL_FALSE, &proj[0][0]);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, WIDTH, HEIGHT);


	while (glfwWindowShouldClose(m_window) == GL_FALSE) {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glViewport(0, 0, WIDTH, HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		OUTPUT_GLERROR;

		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		OUTPUT_GLERROR;


		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, WIDTH, HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		OUTPUT_GLERROR;


		glfwSwapBuffers(m_window);

		glfwWaitEvents();
		OUTPUT_GLERROR;
	}
}

}
