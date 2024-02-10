#ifndef POINT_CLOUD_IO
#define POINT_CLOUD_IO
class PointCloud;
class PointCloudIO
{
public:
	PointCloudIO() {};
	~PointCloudIO() {};

	static PointCloud* Load(const String& name);
	static PointCloud* Create2D(int poositionNum, const glm::vec2& min, const glm::vec2& max);

	static void OutputText(PointCloud* pPointCloud, const String& name);
	static void OutputBinary(PointCloud* pPointCloud, const String& name);

private:
	static void LoadBin(PointCloud* pPointCloud, const String& name);
	static void LoadXYZ(PointCloud* pPointCloud, const String& name);
	static void LoadPCD(PointCloud* pPointCloud, const String& name);
};


#endif POINT_CLOUD_IO