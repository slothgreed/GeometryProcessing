#ifndef GEOMETRY_UTILITY_H
#define GEOMETRY_UTILITY_H
#include "BDB.h"
namespace KI
{
typedef std::pair<int, unsigned int> TriangleToMorton;
class MortonCode
{
public:
	MortonCode() {};
	~MortonCode() {};

	struct Data
	{
		Data()
			: triangleIndex(-1)
			, morton(-1)
		{
		}
		BDB box;
		int triangleIndex;
		int morton;
	};


	void Create(const Vector<Vector3>& triangle, const Vector<unsigned int>& trinagle, const BDB& box, int resolute = 1024);
	static unsigned int To(unsigned int x);
	static Vector3 ToColor(unsigned int morton);
	const Vector<Data>& Get() const { return m_mortons; }
private:
	Vector<Data> m_mortons;
};

}



#endif GEOMETRY_UTILITY_H
