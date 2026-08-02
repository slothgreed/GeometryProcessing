#ifndef OFF_LOADER_H
#define OFF_LOADER_H
namespace KI
{
class Mesh;
class GeometryLoader
{
public:
	GeometryLoader() {};
	~GeometryLoader() {};

	static Mesh* LoadOff(const String& name);

private:

};
}
#endif OFF_LOADER_H