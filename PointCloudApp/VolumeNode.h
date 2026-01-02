#ifndef VOLUME_NODE_H
#define VOLUME_NODE_H
#include "RenderNode.h"
#include "IShader.h"
#include "Texture.h"
#include "Voxel.h"
#include "Primitives.h"
namespace KI
{
class VolumeNode : public RenderNode
{
public:

	class VolumeImageShader : public IShadingShader
	{
	public:
		VolumeImageShader();
		~VolumeImageShader();

		virtual ShaderPath GetShaderPath();
		virtual void FetchUniformLocation();

		void SetCamera(const GLBuffer* pBuffer);
		void SetModel(const Matrix4x4& value) override;
		void SetPosition(GLBuffer* pPosition);
		void SetPosition(PlanePrimitive::Axis axis, float position);
		void SetTexture(Texture* pTexure);
		void SetTexcoord(GLBuffer* pTexture);
	private:
		GLuint m_uModel;
		GLuint m_uTexture;
		GLuint m_uPosition;
	};

	class VolumeRayCastShader : public IShadingShader
	{
	public:
		VolumeRayCastShader() {};
		~VolumeRayCastShader() {};

		virtual ShaderPath GetShaderPath();
		virtual void FetchUniformLocation();
		void SetCamera(const GLBuffer* pBuffer);
		void SetVoxel(const GLBuffer* pBuffer);
		void SetPosition(GLBuffer* pPosition);
		void SetModel(const Matrix4x4& value);
		void SetTexture(Texture* pTexture);

	private:
		GLuint m_uModel;
		GLuint m_uInvModel;
		GLuint m_uTexture;
		GLuint m_uStepSize;
	};


	class MarchingCubeShader : public IMeshShader
	{
	public:
		virtual int GetTaskThreadNum() const { return 32; }
		MarchingCubeShader(bool withTask = false)
			: m_withTask(withTask)
			, m_uTaskNum(0) {};
		~MarchingCubeShader() {};

		virtual ShaderPath GetShaderPath();
		virtual void FetchUniformLocation();

		bool WithTask() const { return m_withTask; }
		void SetCamera(const GLBuffer* pBuffer);
		void SetVoxel(const GLBuffer* pBuffer);
		void SetVoxelData(const GLBuffer* pBuffer);
		void SetTriTable(const GLBuffer* pBuffer);
		void SetCubeIndex(const GLBuffer* pBuffer);
		void SetTaskNums(const GLBuffer* pBuffer);
		void SetTaskNum(unsigned int value);
		void SetModel(const Matrix4x4& value);
		void SetThreshold(float value);
	private:
		bool m_withTask;
		GLuint m_uModel;
		GLuint m_uThreshold;
		GLuint m_uTaskNum;
	};


	VolumeNode(Unique<Voxel>&& pVoxel);
	~VolumeNode();

	virtual void Draw(const DrawContext& context);
	virtual void ShowUI(UIContext& ui);

private:
	void BuildResource();
	void BuildVoxelResource(bool withData);

	struct UI
	{
		struct Plane
		{
			Plane()
				: visible(false)
				, position(0.0f)
			{
			}
			bool visible;
			float position;
			Matrix4x4 matrix;
		};

		struct MarchingCube
		{
			MarchingCube()
				: visible(false)
				, useMeshShader(false)
				, isolate(0.0f)
			{
			}
			bool visible;
			bool useMeshShader;
			float isolate;
		};

		struct RayCast
		{
			RayCast()
				: visible(true)
			{
			}

			bool visible;
		};

		UI() 
			: visible(true) 
			, visibleBDB(false) {};
		Plane xPlane;
		Plane yPlane;
		Plane zPlane;
		MarchingCube marching;
		RayCast rayCast;
		bool visible;
		bool visibleBDB;
	};

	UI m_ui;

	struct Gpu
	{
		Unique<GLBuffer> pBDBLine;
		Unique<GLBuffer> pBDBLineIndex;

		Unique<GLBuffer> pPlane;
		Unique<GLBuffer> pPlaneTex;
		Unique<GLBuffer> pPlaneIndex;

		Unique<GLBuffer> pTriTable;
		Unique<GLBuffer> pVoxel;
		Unique<GLBuffer> pVoxelData;
		Unique<GLBuffer> pCubeIndexs; // useTaskShader;
		Unique<GLBuffer> pTaskNum; // useTaskShader;
	};

	Gpu m_gpu;
	Unique<Texture3D> m_pTexture3D;
	Unique<Voxel> m_pVoxel;
	MarchingCube m_marching;
	Unique<VolumeNode::VolumeRayCastShader> m_pRayCastShader;
	Unique<VolumeNode::MarchingCubeShader> m_pMarchingShader;
	Unique<VolumeNode::VolumeImageShader> m_pVolumeImageShader;
};


}
#endif VOLUME_NODE_H
