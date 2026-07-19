#ifndef SIGNED_DISTANCE_FIELD
#define SIGNED_DISTANCE_FIELD
#include "IAlgorithm.h"
#include "RenderResource.h"
#include "BVH.h"
#include "Voxel.h"
namespace KI
{
class HalfEdgeNode;
class SignedDistanceField : public IAlgorithm
{
public:
	SignedDistanceField(HalfEdgeNode* pNode);
	~SignedDistanceField();

	virtual ALGORITHM_TYPE GetType() override { return ALGORITHM_SIGNED_DISTANCE_FIELD; }
	virtual void Execute();
	virtual void ShowUI(RenderNode* pNode, UIContext& ui);

	VoxelF CreateSDFData(int resolute);
	void UpdateInnerOuter(VoxelF& voxel);
private:

	enum Axis
	{
		X, Y, Z
	};

	class Shader : public IComputeShader
	{
	public:

		enum Type
		{
			FETCH,
			TO_IMAGE
		};


		Shader(Type type) : m_type(type) {};
		virtual ~Shader() {};

		virtual Vector3i GetLocalThreadNum() const;
		virtual ShaderPath GetShaderPath();
		virtual void FetchUniformLocation();
		void Execute(HalfEdgeNode* pNode, int resolute, Axis axis, float position, Texture2D* pTexture, float frequency, GLBuffer* pDebugBuffer);
		void Execute(HalfEdgeNode* pNode, int resolute, GLBuffer* pBuffer);

	private:
		GLuint m_minBox = -1;
		GLuint m_maxBox = -1;
		GLuint m_pitch = -1;
		GLuint m_position = -1;
		GLuint m_axis = -1;
		GLuint m_resolute = -1;
		GLuint m_maxTriangle = -1;
		GLuint m_frequency = -1;
		GLuint m_model = -1;
		Type m_type = Type::FETCH;
	};




	float CalcMinDistance(const Vector3& pos) const;
	void CreateTexure(int resolute);
	bool DebugSDFCPUGPU(int resolute);
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
		};
		UI();
		Shared<GLStatus> glStatus;
		int resolute = 4;
		Plane xPlane;
		Plane yPlane;
		Plane zPlane;
	};

	struct Gpu
	{
		Shared<Texture2D> xTexture;
		Shared<Texture2D> yTexture;
		Shared<Texture2D> zTexture;
	};

	Gpu m_gpu;
	UI m_ui;
	HalfEdgeNode* m_pHalfEdge;
	float m_frequency;
	float m_resolute;
	Unique<Shader> m_pShader;

};

}

#endif SIGNED_DISTANCE_FIELD
