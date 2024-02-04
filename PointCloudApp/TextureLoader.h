#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H
class Texture;
class TextureLoader
{
public:
	TextureLoader();
	~TextureLoader();

	static Texture* Load(const std::string& name);
private:

};

#endif TEXTURE_LOADER_H

