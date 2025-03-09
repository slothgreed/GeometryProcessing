#include "TextureLoader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Texture.h"
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

Texture* TextureLoader::Load(const String& name)
{
	auto pTexture = new Texture2D();
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

Texture* TextureLoader::Create(const Vector2i& resolute)
{
	auto pTexture = new Texture2D();
	unsigned char* pixelData = new unsigned char[resolute.x * resolute.y * 4];
	int i = 0;
	for (int i = 0; i < resolute.x; i++) {
		for (int j = 0; j < resolute.y; j++) {
			pixelData[i * 4] = 255;
			pixelData[i * (4 + 1)] = 0;
			pixelData[i * (4 + 2)] = 0;
			pixelData[i * (4 + 3)] = 0;
		}
	}

	pTexture->Build(resolute.x, resolute.y, pixelData);
	delete[] pixelData;
	return pTexture;
}

}