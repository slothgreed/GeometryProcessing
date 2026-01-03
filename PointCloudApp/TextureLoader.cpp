#include "TextureLoader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Texture.h"
#include "FileUtility.h"
#include "Utility.h"
namespace KI
{
TextureLoader::PixelData::PixelData()
	: width(0)
	, height(0)
	, component(0)
	, data(nullptr)
{
}
TextureLoader::PixelData::~PixelData()
{
	if (data) {
		stbi_image_free(data);
		data = nullptr;
	}
}
TextureLoader::TextureLoader()
{
}

TextureLoader::~TextureLoader()
{
}

Texture* TextureLoader::Load(const String& name, bool useMipmap)
{
	auto pTexture = new Texture2D();
	pTexture->UseMipmap();
	int x, y, n;
	auto data = stbi_load(name.data(), &x, &y, &n, 4);
	pTexture->Build(x, y, data);
	stbi_image_free(data);
	return pTexture;
}

TextureLoader::PixelData* TextureLoader::LoadData(const String& name, int comp)
{
	auto pixel = new PixelData();
	pixel->data = stbi_load(name.data(), &pixel->width, &pixel->height, &pixel->component, comp);
	return pixel;
}
Voxel* TextureLoader::LoadVolume(const String& name)
{
	/*
	{
		const int sizeX = 5;
		const int sizeY = 5;
		const int sizeZ = 5;
		std::vector<unsigned short> volume(sizeX * sizeY * sizeZ);

		Vector3 center(sizeX / 2.0f, sizeY / 2.0f, sizeZ / 2.0f);
		float radius = 1.5f;

		for (int z = 0; z < sizeZ; ++z) {
			for (int y = 0; y < sizeY; ++y) {
				for (int x = 0; x < sizeX; ++x) {
					Vector3 pos((float)x, (float)y, (float)z);
					float dist = glm::length(pos - center);

					unsigned short value = (dist <= radius) ? 65535 : 0;
					volume[x + y * sizeX + z * sizeX * sizeY] = value;
				}
			}
		}
		return new Voxel(Vector3(sizeX, sizeY, sizeZ), BDB(Vector3(0), Vector3(1)), std::move(volume));
	}
	*/
	
	FileReader reader;
	reader.Open(name, true);
	auto size =  reader.ReadVector<unsigned short>(3);
	std::vector<unsigned short> volume(size[0] * size[1] * size[2]);
	for (int z = 0; z < size[2]; z++) {
		for (int y = 0; y < size[1]; y++) {
			reader.ReadVector<unsigned short>(&volume[y * size[0] + z * size[0] * size[1]],size[0]);
		}
	}

	return new Voxel(Vector3(size[0], size[1], size[2]), BDB(Vector3(0), Vector3(1)), std::move(volume));
}
}