#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H
namespace KI
{
class Texture;
class TextureLoader
{
public:
	TextureLoader();
	~TextureLoader();

	static Texture* Load(const String& name);
private:

};
}
#endif TEXTURE_LOADER_H

