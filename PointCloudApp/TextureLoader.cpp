#include "TextureLoader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Texture.h"
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
	return pTexture;
}

