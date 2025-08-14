#ifndef VOXEL_H
#define VOXEL_H
#include "BDB.h"
#include "GLBuffer.h"
namespace KI
{

class Voxel
{
public:
	Voxel(int resolute, const BDB& bdb);
	Voxel(const Vector3i& size, const BDB& bdb, std::vector<unsigned short>&& data);
	~Voxel() {};

	Vector3 GetPosition(const Vector3i& data) const;
	unsigned short GetData(const Vector3i& data) const;
	Vector<unsigned short> GetDatas() const { return m_ushort; }
	int GetIndex(const Vector3i& data) const;
	int GetIndex(int x, int y, int z) const;
	const Vector3i& GetResolute() const { return m_resolute; }
	const BDB& GetBDB() const { return m_bdb; }
	Vector<Vector4> CreateGrayScale() const;
	int GetSize() const { return m_resolute.x * m_resolute.y * m_resolute.z; }
	struct Gpu
	{
		Vector4 pitch;
		Vector4 bdbMin;
		Vector4 bdbMax;
		Vector4i resolute;
	};

	Gpu CreateGpuInfo() const;

private:
	Vector3 m_pitch;
	BDB m_bdb;
	Vector3i m_resolute;
	Vector<unsigned short> m_ushort;
};

class MarchingCube
{
public:
	MarchingCube() {};
	~MarchingCube() {};

	const GLBuffer* GetPositionBuffer() const { return m_gpu.pPosition.get(); }
	const GLBuffer* GetNormalBuffer() const { return m_gpu.pNormal.get(); }
	void Build(const Voxel& voxel, float threshold);
	Vector<int> CreateFlattenTriangleTable() const;
private:
	Vector3 VertexInterp(float isolevel, const Vector3& p1, const Vector3& p2, float valp1, float valp2);
	struct Gpu
	{
		Unique<GLBuffer> pPosition;
		Unique<GLBuffer> pNormal;
	};



	Gpu m_gpu;
};


}

#endif VOXEL_H
