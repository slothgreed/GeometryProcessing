#ifndef SIGNED_DISTANCE_FIELD
#define SIGNED_DISTANCE_FIELD
#include "IAlgorithm.h"
#include "RenderResource.h"
#include "BVH.h"
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
	virtual void ShowUI(UIContext& ui);
private:

	enum Axis
	{
		X, Y, Z
	};

	class Shader : public IComputeShader
	{
	public:
		Shader() {};
		virtual ~Shader() {};

		virtual Vector3i GetLocalThreadNum() const { return Vector3i(32, 32, 1); }
		virtual ShaderPath GetShaderPath();
		virtual void FetchUniformLocation();
		void Execute(const HalfEdgeNode* pNode, int resolute, Axis axis, float position, Texture2D* pTexture, float frequency, GLBuffer* pDebugBuffer);
		enum UNIFORM
		{
			MINBOX,
			PITCH,
			POSITION,
			AXIS,
			RESOLUTE,
			MAXTRIANGLE,
			FREQUENCY,
			MODEL,
			NUM
		};

	private:
		GLuint m_uniform[UNIFORM::NUM];
	};




	float CalcMinDistance(const Vector3& pos) const;
	void CreateTexure(int resolute);
	void CreateSDFTexture(int resolute, Axis axis, float position, Texture2D* pTexture);

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
		int resolute;
		Plane xPlane;
		Plane yPlane;
		Plane zPlane;
	};

	struct Gpu
	{
		Unique<GLBuffer> pDebugBuffer;
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
