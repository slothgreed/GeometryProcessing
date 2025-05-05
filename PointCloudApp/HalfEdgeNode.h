#ifndef KI_MESH_NODE_H
#define KI_MESH_NODE_H
#include "SimpleShader.h"
#include "RenderNode.h"
#include "HalfEdgeStruct.h"
#include "BVH.h"
#include "MeshletGenerator.h"
#include "GeometryUtility.h"
namespace KI
{
class BVH;
class Voxelizer;
class ShapeDiameterFunction;
class SignedDistanceField;
class HalfEdgeNode : public RenderNode
{
public:

	struct HalfEdgeParts : public RenderParts
	{
	public:
		enum Type
		{
			Face,
			Edge,
			Vertex
		};


		HalfEdgeParts(Type _type, int _parts)
		:type(_type)
		,parts(_parts){};
		~HalfEdgeParts() {};
		static const HalfEdgeParts* Cast(const RenderParts* pParts);
		virtual String ToString();
		Type type;
		int parts;
	};


	HalfEdgeNode(const String& name, const Shared<HalfEdgeStruct>& pStruct);
	~HalfEdgeNode();

	HalfEdgeStruct* GetData() { return m_pHalfEdge.get(); }
	const HalfEdgeStruct* GetData() const { return m_pHalfEdge.get(); }

	const BVH* GetBVH() const { return m_pBVH; }
	const MortonCode& GetMorton() const { return m_morton.data; }
	GLBuffer* GetPositionGpu() { return m_gpu.position.get(); }
	GLBuffer* GetFaceIndexGpu() { return m_gpu.faceIndexBuffer.get(); }
protected:
	virtual void ShowUI(UIContext& ui);
	virtual void DrawNode(const DrawContext& context);
	virtual void PickNode(const PickContext& context);
	virtual void DrawPartsNode(const DrawContext& context, const RenderParts& parts);
	virtual bool CollectPickedNode(PickResult& result);
	virtual void UpdateData(float time) {};
	
private:
	void BuildBVH();
	void BuildMorton();
	void BuildEdge();
	void ShowNormal();
	void BuildGLBuffer();

	Unique<VertexVectorShader> m_pVectorShader;

	struct PickId
	{
		int begin;
		int num;

		bool Inner(int id)
		{
			return begin <= id && id < begin + num;
		}
	};

	struct PickIds
	{
		PickId face;
		PickId edge;
		PickId vertex;
	};
	struct GeometryGpu
	{
		GeometryGpu()
			: position(nullptr)
			, normal(nullptr)
			, vertexColor(nullptr)
			, faceIndexBuffer(nullptr)
			, edgeIndexBuffer(nullptr)
		{
		}
		Unique<GLBuffer> position;
		Unique<GLBuffer> normal;
		Unique<GLBuffer> vertexColor;
		Unique<GLBuffer> faceIndexBuffer;
		Unique<GLBuffer> edgeIndexBuffer;
	};

	GeometryGpu m_gpu;
	Shared<HalfEdgeStruct> m_pHalfEdge;
	
	struct MeshletGpu
	{
		Unique<MeshletShader> shader;
		Unique<GLBuffer> color;
		Unique<GLBuffer> position;
		Unique<GLBuffer> culster;
		Unique<GLBuffer> index;
	};


	PickIds m_pickIds;

	MeshletGpu m_meshletGpu;

	struct Morton
	{
		Unique<GLBuffer> gpuLine;
		Unique<GLBuffer> gpuColor;
		MortonCode data;
	};

	Morton m_morton;
	BVH* m_pBVH;
	SignedDistanceField* m_pSignedDistanceField;
	ShapeDiameterFunction* m_pShapeDiameterFunction;
	Voxelizer* m_pVoxelizer;

	struct UI
	{
		struct Poisson
		{
			struct Surface
			{
				Surface()
					: create(false)
					, num(10000)
					, length(0.1f)
				{
				}

				bool create;
				int num;
				float length;
			};
			Poisson()
				: volume(false)
			{
			}
			bool volume;
			Surface surface;
		};

		struct HeatMethod
		{
			HeatMethod() 
				: timeStep(1.0f) {}
			float timeStep;
		};

		struct Voxel
		{
			Voxel() :visible(false), resolute(8) {}
			bool visible;
			int resolute;
		};
		UI()
			: visible(true)
			, visibleBVH(false)
			, visibleMeshlet(false)
			, meshlet(0)
			, visibleMesh(true)
			, visibleEdge(false)
			, visibleVertex(false)
			, visibleNormal(false)
			, visibleMorton(false)
			, visibleSignedDistanceField(false)
			, normalLength(1.0f)
			, vertexParameter(0)
		{
		}
		bool visible;
		bool visibleBVH;
		bool visibleMeshlet;
		bool visibleMorton;
		bool visibleSignedDistanceField;
		int meshlet;
		bool visibleMesh;
		bool visibleEdge;
		bool visibleVertex;
		bool visibleNormal;
		float normalLength;
		int vertexParameter;
		Voxel voxel;
		HeatMethod heatMethod;
		Poisson poisson;
	};

	Parameter m_vertexParameter;
	UI m_ui;
};


}


#endif KI_MESH_NODE_H