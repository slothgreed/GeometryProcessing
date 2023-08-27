#ifndef UTILITY_H
#define UTILITY_H

class ColorUtility
{
public:
	ColorUtility() {};
	~ColorUtility() {};

	static vec3 CreateRandom();
	static vec3 CreatePseudo(unsigned int value, unsigned int maxValue);
private:
	static void InitializePseudoColor();
};

#endif UTILITY_H
