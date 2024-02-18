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
	void SetColor(Vector<Vector3>&& color) { m_color = std::move(color); }
	const String& GetFileName() const { return m_filePath; }
	void To2D();
	Vector<glm::vec4> CreatePosition4f();
	Vector<glm::vec4> CreateColor4f();
private:

	String m_filePath;
	Vector<float> m_curvature;
};

}


#endif POINT_CLOUD