#ifndef POINT_CLOUD
#define POINT_CLOUD
#include "Primitive.h"
class PointCloud : public Primitive
{
public:

	PointCloud();
	~PointCloud();
	static PointCloud* Load(const std::string& name);

	void SetColor(std::vector<glm::vec3>&& color) { m_color = std::move(color); }
	const std::string& GetFileName() { return m_filePath; }
	static PointCloud* Create2D(int poositionNum, const glm::vec2& min, const glm::vec2& max);
	void OutputText(const std::string& name);
	void OutputBinary(const std::string& name);
private:
	void LoadBin(const std::string& name);
	void LoadXYZ(const std::string& name);
	void LoadPCD(const std::string& name);
	std::string m_filePath;
	std::vector<float> m_curvature;
};


#endif POINT_CLOUD