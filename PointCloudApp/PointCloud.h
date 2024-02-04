#ifndef POINT_CLOUD
#define POINT_CLOUD
#include "Primitive.h"
class PointCloud : public Primitive
{
public:
	friend class PointCloudIO;
	PointCloud();
	~PointCloud();
	void SetColor(std::vector<glm::vec3>&& color) { m_color = std::move(color); }
	const std::string& GetFileName() const { return m_filePath; }
	void To2D();
	std::vector<glm::vec4> CreatePosition4f();
	std::vector<glm::vec4> CreateColor4f();
private:

	std::string m_filePath;
	std::vector<float> m_curvature;
};


#endif POINT_CLOUD