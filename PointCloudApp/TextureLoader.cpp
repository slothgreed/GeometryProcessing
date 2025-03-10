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


}