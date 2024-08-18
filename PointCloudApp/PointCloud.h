#ifndef POINT_CLOUD
#define POINT_CLOUD
#include "Primitive.h"
namespace KI
{

class PointCloud : public Primitive
{
public:
	friend class PointCloudIO;
	PointCloud();
	~PointCloud();
	const String& GetFileName() const { return m_filePath; }
	void To2D();
	static Vector<Vector3> Create3D(int size, const Vector3& min, const Vector3& max);
	void SetColorSingle(const Vector3& color);
	Vector<Vector4> CreatePosition4f();
	Vector<Vector4> CreateColor4f();
	Vector<Vector4> CreatePositionColor4f();
private:

	String m_filePath;
	Vector<float> m_curvature;
};

}


#endif POINT_CLOUD