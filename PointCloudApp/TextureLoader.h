#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H
#include "Voxel.h"
namespace KI
{
class Texture;
class TextureLoader
{
public:

	struct PixelData
	{
		PixelData();
		~PixelData();
		int width;
		int height;
		int component;
		unsigned char* data;
	};


	TextureLoader();
	~TextureLoader();

	static Texture* Load(const String& name, bool useMipmap);
	static PixelData* LoadData(const String& name, int comp);
	static Voxel* LoadVolume(const String& name);
private:

};
}
#endif TEXTURE_LOADER_H

