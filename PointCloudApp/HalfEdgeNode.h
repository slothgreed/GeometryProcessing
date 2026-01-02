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
class HalfEdgeController;
class HalfEdgeNode : public RenderNode
{
public:

	struct HalfEdgeParts : public RenderParts
	{
	public:
		enum class Type
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
		int IsFace() const { return type == Type::Face; }
		int IsEdge() const { return type == Type::Edge; }
		int IsVertex() const { return type == Type::Vertex; }
		Type type;
		int parts;
	};


	HalfEdgeNode(const String& name, const Shared<HalfEdgeStruct>& pStruct);
	~HalfEdgeNode();

	HalfEdgeStruct* GetData() { return m_pHalfEdge.get(); }
	const HalfEdgeStruct* GetData() const { return m_pHalfEdge.get(); }

	BVH* GetBVH();
	const MortonCode& GetMorton() const { return m_morton.data; }
	GLBuffer* GetPositionGpu() const { return m_gpu.position.get(); }
	GLBuffer* GetFaceIndexGpu() const { return m_gpu.faceIndexBuffer.get(); }
	GLBuffer* GetBVHGpu();

	void UpdateVertex();

	int ToPickFaceIndex(int index) const
	{
		return index - m_pickIds.face.begin;
	}

	int ToPickEdgeIndex(int index) const
	{
		return index - m_pickIds.edge.begin;
	}

	int ToPickVertexIndex(int index) const
	{
		return index - m_pickIds.vertex.begin;
	}
	virtual void ProcessMouseEvent(const PickContext& context);
protected:
	virtual void ShowUI(UIContext& ui);
	virtual void DrawNode(const DrawContext& context);
	virtual void PickNode(const PickContext& context);
	virtual void DrawPartsNode(const DrawContext& context, const RenderParts& parts);
	virtual bool CollectPickedNode(PickResult& result);
	virtual void UpdateData(float time) {};
	
private:

	enum UPDATE_DATA
	{
		UPDATE_VERTEX,
		UPDATE_NUM,
	};


	std::bitset<UPDATE_NUM> m_updateData;
	bool m_updateVertex;
	void BuildBVH();
	void BuildMorton();
	void BuildEdge();
	void ShowNormal(const DrawContext& context);
	void BuildGLBuffer();

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
		Unique<GLBuffer> vertexDir1;
		Unique<GLBuffer> vertexDir2;
		Unique<GLBuffer> faceIndexBuffer;
		Unique<GLBuffer> edgeIndexBuffer;
		Unique<GLBuffer> bvh;
	};

	GeometryGpu m_gpu;
	Shared<HalfEdgeStruct> m_pHalfEdge;
	
	struct MeshletGpu
	{
		Unique<MeshletShader> shader;
		Unique<GLBuffer> position;
		Unique<GLBuffer> cluster;
		Unique<GLBuffer> taskNum; // TaskShaderì‡Ç≈é¿çsÇ∑ÇÈMeshletÇÃêîÇäiî[Çµ,MeshShaderÇ≈èàóùÇ∑ÇÈîzóÒ
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

		struct Meshlet
		{
			Meshlet() :visible(false), level(7), cullSize(5) {}
			bool visible;
			int level;
			int cullSize;
		};

		struct MST
		{
			MST() : visible(false), weight(1.0f) {}
			bool visible;
			float weight;
		};

		UI()
			: visible(true)
			, visibleBVH(false)
			, visibleMesh(true)
			, visibleEdge(false)
			, visibleVertex(false)
			, visibleNormal(false)
			, visibleMorton(false)
			, editVertex(false)
			, visibleSignedDistanceField(false)
			, normalLength(1.0f)
			, vertexValue(0)
			, vertexDirection(0)
		{
		}
		bool visible;
		bool visibleBVH;
		bool visibleMorton;
		bool visibleSignedDistanceField;
		bool visibleMesh;
		bool visibleEdge;
		bool visibleVertex;
		bool visibleNormal;
		bool editVertex;
		float normalLength;
		int vertexValue;
		int vertexDirection;
		MST mst;
		Meshlet meshlet;
		Voxel voxel;
		HeatMethod heatMethod;
		Poisson poisson;
	};
	Shared<HalfEdgeParts> m_pEditVertex;
	HalfEdgeController* m_pController;
	Unique<MeshletProfiler> m_meshletProfiler;
	Parameter m_vertexParameter;
	UI m_ui;
};


}


#endif KI_MESH_NODE_H