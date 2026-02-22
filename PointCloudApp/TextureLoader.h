#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H
#include "Voxel.h"
#include "Texture.h"
namespace KI
{
class Texture;
class TextureLoader
{
public:

	
	TextureLoader();
	~TextureLoader();

	static Texture* Load(const String& name, bool useMipmap);
	static PixelData* LoadData(const String& name, int comp);
	static Voxel* LoadVolume(const String& name);
	static Texture* LoadPGM(const String& name, bool hasCPU);

private:

};
}
#endif TEXTURE_LOADER_H

