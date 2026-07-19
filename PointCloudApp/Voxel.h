#ifndef VOXEL_H
#define VOXEL_H
#include "BDB.h"
#include "GLBuffer.h"
#include "Mesh.h"
namespace KI
{
enum VOXEL_LABEL
{
	VOXEL_LABEL_UNKNOWN = 0,
	VOXEL_LABEL_INNER = 1,
	VOXEL_LABEL_BOUNDARY = 2,
	VOXEL_LABEL_OUTER = 3,
	VOXEL_LABEL_NUM
};
template<typename T>
class Voxel
{
public:
	Voxel(int resolute) { m_resolute = Vector3i(resolute, resolute, resolute); }
	Voxel(const Vector3i& resolute) { m_resolute = resolute; }
	Voxel(int resolute, const BDB& bdb) { m_resolute = Vector3i(resolute, resolute, resolute); m_bdb = bdb; }
	Voxel(const Vector3i& resolute, const BDB& bdb, std::vector<T>&& data) { m_resolute = resolute; m_bdb = bdb; m_data = std::move(data); }
	~Voxel() {};



	void Allocate() { m_data.resize(GetSize()); }
	Vector3 GetPosition(const Vector3i& data) const { return Vector3(data.x, data.y, data.z) * GetPitch() + m_bdb.Min(); }
	void SetData(const std::vector<T>&& data) { m_data = std::move(data); }
	void SetData(const Vector3i& index, const T& data) { m_data[GetIndex(index)] = data; }
	T GetData(const Vector3i& data) const { return m_data[GetIndex(data)]; }
	T GetData(int x, int y, int z) const { return m_data[GetIndex(x, y, z)]; }
	int GetIndex(const Vector3i& data) const { return GetIndex(data.x, data.y, data.z); }
	int GetIndex(int x, int y, int z) const { return x + y * m_resolute.x + z * m_resolute.x * m_resolute.y; }
	const Vector3i& GetResolute() const { return m_resolute; }
	const BDB& GetBDB() const { return m_bdb; }
	int GetSize() const { return m_resolute.x * m_resolute.y * m_resolute.z; }
	Vector3 GetPitch() const { return  (m_bdb.Max() - m_bdb.Min()) / Vector3(m_resolute); }
	const std::vector<T>& GetData() const { return m_data; }
	bool In(const Vector3i& index)
	{
		return
			index.x >= 0 && index.x < m_resolute.x &&
			index.y >= 0 && index.y < m_resolute.y &&
			index.z >= 0 && index.z < m_resolute.z;
	}
	Vector3 GetCenter(const Vector3i& index) const
	{
		auto pitch = GetPitch();
		return Vector3(
			pitch.x * index.x,
			pitch.y * index.y,
			pitch.z * index.z) + GetBDB().Min()
			+ (pitch / 2.0f);
	}

	class Serializer
	{
	public:
		Serializer() {};
		~Serializer() {};
		static bool Save(const String& path, const Voxel<T>& voxel)
		{
	        std::ofstream ofs(path, std::ios::binary);

			float min[3] =
			{
				voxel.GetBDB().Min().x,
				voxel.GetBDB().Min().y,
				voxel.GetBDB().Min().z
			};

			float max[3] =
			{
				voxel.GetBDB().Max().x,
				voxel.GetBDB().Max().y,
				voxel.GetBDB().Max().z
			};

			int resolution[3] =
			{
				voxel.GetResolute().x,
				voxel.GetResolute().y,
				voxel.GetResolute().z
			};

			ofs.write(reinterpret_cast<char*>(min), sizeof(min));
			ofs.write(reinterpret_cast<char*>(max), sizeof(max));
			ofs.write(reinterpret_cast<char*>(resolution), sizeof(resolution));
			ofs.write(
				reinterpret_cast<const char*>(voxel.m_data.data()),
				voxel.m_data.size() * sizeof(T));

			return true;
		}
	private:
	};


private:
	BDB m_bdb;
	Vector3i m_resolute;
	Vector<T> m_data;
};

using VoxelByte = Voxel<char>;
using VoxelU16 = Voxel<unsigned short>;
using VoxelF = Voxel<float>;
class MarchingCube
{
public:
	MarchingCube() {};
	~MarchingCube() {};
	Mesh CreateMesh(const VoxelU16& voxel, float threshold);
	Vector<int> CreateFlattenTriangleTable() const;
private:
	Vector3 VertexInterp(float isolevel, const Vector3& p1, const Vector3& p2, float valp1, float valp2);

};

class DualContouring
{
public:
	DualContouring() {};
	~DualContouring() {};

	struct Mesh
	{
		Vector<Vector3> position;
		Vector<Vector3> normal;
		Vector<UInt> indices;
	};

	Mesh CreateMesh(const VoxelF& voxel);

private:
	struct Hermite
	{
		Hermite() : edgeIndex(-1) {}
		Vector3 position;
		Vector3 normal;
		int edgeIndex;
		bool IsValidate() const
		{
			return edgeIndex != -1;
		}
	};


	struct CellEdgeHermite
	{
		std::array<Hermite,12> data;

		Vector3 CalcPosition() const;
		Vector3 CalcNormal() const;
		bool IsValid() const
		{
			for (size_t i = 0; i < data.size(); i++) {
				if (data[i].IsValidate()) { return true; }
			}
			return false;
		}
	};



	struct CellNeighbor8
	{
		std::array<Vector3, 8> positions;
		std::array<float, 8> distances;
		bool HasBoundary() const
		{
			bool hasInner = false;
			bool hasOuter = false;
			for (int i = 0; i < 8; i++) {
				if (distances[i] < 0) {
					hasInner = true;
				} else {
					hasOuter = true;
				}
			}
			return hasInner && hasOuter;
		}
	};

	// voxelのセルの辺のインデックスを定義する配列
	static constexpr std::array<std::pair<int,int>,12> m_cellEdges =
	{
		std::pair<int,int>(0,1),
		std::pair<int,int>(1,2),
		std::pair<int,int>(2,3),
		std::pair<int,int>(3,0),
		std::pair<int,int>(4,5),
		std::pair<int,int>(5,6),
		std::pair<int,int>(6,7),
		std::pair<int,int>(7,4),
		std::pair<int,int>(0,4),
		std::pair<int,int>(1,5),
		std::pair<int,int>(2,6),
		std::pair<int,int>(3,7)
	};

	static constexpr std::array<Vector3i, 8> m_cornerOffset =
	{
		Vector3i{0, 0, 0}, // 0
		Vector3i{1, 0, 0}, // 1
		Vector3i{1, 1, 0}, // 2
		Vector3i{0, 1, 0}, // 3
		Vector3i{0, 0, 1}, // 4
		Vector3i{1, 0, 1}, // 5
		Vector3i{1, 1, 1}, // 6
		Vector3i{0, 1, 1}, // 7
	};

	CellNeighbor8 GetCellNeighbor8(const VoxelF& voxel, const Vector3i& cellIndex) const;
	Vector3 CalcIntersection(const Vector3& p1, const Vector3& p2, float valp1, float valp2) const;
	float GetSDF(const VoxelF& voxel, const Vector3& position) const;
	CellEdgeHermite CreateCellEdgeHermite(const VoxelF& voxel, const Vector3i& index) const;
	bool CalcNormal(const VoxelF& voxel, const Vector3& position, Vector3& result) const;
	Vector3 CalcGradient(const VoxelF& voxel, const Vector3& position) const;



	bool IsInside(float sdf) const { return sdf < 0.0f; }
	bool IsBoundary(float sdf0, float sdf1) const { return IsInside(sdf0) != IsInside(sdf1); }
};

}

#endif VOXEL_H
