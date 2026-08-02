#ifndef VOLUME_NODE_H
#define VOLUME_NODE_H
#include "RenderNode.h"
#include "IShader.h"
#include "Texture.h"
#include "Voxel.h"
#include "Primitives.h"
#include "AlgorithmShader.h"
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




	VolumeNode(Unique<VoxelU16>&& pVoxel);
	~VolumeNode();

	virtual void Draw(const DrawContext& context);
	virtual void ShowUI(UIContext& ui);

private:
	void BuildResource();
	void BuildVoxelResource(bool withData);
	Vector<Vector4> CreateGrayScale(const VoxelU16& voxel) const;

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

		Unique<GLBuffer> pMarchingPosition;
		Unique<GLBuffer> pMarchingNormal;
		Unique<GLBuffer> pVoxel;
		Unique<GLBuffer> pVoxelData;
	};

	Gpu m_gpu;
	Unique<Texture3D> m_pTexture3D;
	Unique<VoxelU16> m_pVoxel;
	Unique<VolumeNode::VolumeRayCastShader> m_pRayCastShader;
	Unique<MarchingCubeShader> m_pMarchingShader;
	Unique<VolumeNode::VolumeImageShader> m_pVolumeImageShader;
};


class VoxelNode : public RenderNode
{
public:

	VoxelNode(const String& name, Unique<VoxelF>&& pVoxel);
	virtual ~VoxelNode();

	virtual void Draw(const DrawContext& context);
	virtual void ShowUI(UIContext& ui);

private:
	void BuildResource();
	void BuildVoxelResource();

	struct UI
	{
		struct MarchingCube
		{
			MarchingCube()
				: visible(false)
				, isolate(0.0f)
			{
			}
			bool visible;
			float isolate;
		};

		UI() {}
		bool visiblePoints = false;
		float minValue = 0.0f;
		float maxValue = 1.0f;
		MarchingCube marching;
	};

	UI m_ui;

	struct Gpu
	{
		Unique<GLBuffer> pPosition;
		Unique<GLBuffer> pColor;
		Unique<GLBuffer> pVoxel;
		Unique<GLBuffer> pVoxelData;
	};

	struct Cache
	{
		float maxValue = -1.0f;
		float minValue = -1.0f;
	};

	Cache m_cache;
	float m_maxValue = 0.0f;
	float m_minValue = 0.0f;
	Gpu m_gpu;
	Unique<VoxelF> m_pVoxel;
	Unique<MarchingCubeShader> m_pMarchingShader;
};


}
#endif VOLUME_NODE_H
