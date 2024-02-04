#ifndef POINT_CLOUD_IO
#define POINT_CLOUD_IO
class PointCloud;
class PointCloudIO
{
public:
	PointCloudIO() {};
	~PointCloudIO() {};

	static PointCloud* Load(const std::string& name);
	static PointCloud* Create2D(int poositionNum, const glm::vec2& min, const glm::vec2& max);

	static void OutputText(PointCloud* pPointCloud, const std::string& name);
	static void OutputBinary(PointCloud* pPointCloud, const std::string& name);

private:
	static void LoadBin(PointCloud* pPointCloud, const std::string& name);
	static void LoadXYZ(PointCloud* pPointCloud, const std::string& name);
	static void LoadPCD(PointCloud* pPointCloud, const std::string& name);
};


#endif POINT_CLOUD_IO