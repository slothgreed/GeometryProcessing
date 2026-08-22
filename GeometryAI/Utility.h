#include <vector>
#include <string>
namespace KI
{

class GeometryUtility
{
public:
	GeometryUtility();
	~GeometryUtility();
	static std::vector<float> NormalizePointCloud(const std::vector<float>& points);
	static std::vector<float> LoadPointCloud(const std::string& filePath);
private:

};
}