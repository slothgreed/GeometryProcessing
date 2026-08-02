#ifndef AI_DATA_GENERATOR_H
#define AI_DATA_GENERATOR_H

namespace KI
{
class Mesh;
class AIDataGenerator
{
public:
	AIDataGenerator();
	~AIDataGenerator();

	void Save(const String& name, const Mesh& mesh, int sampleCount, int generateCount);
private:

};

}

#endif AI_DATA_GENERATOR_H