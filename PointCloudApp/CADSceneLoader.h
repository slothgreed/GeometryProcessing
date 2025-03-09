#ifndef CADSCENE_LOADER_H
#define CADSCENE_LOADER_H

namespace KI
{
class RenderNode;
class CADSceneLoader
{
public:
	CADSceneLoader();
	~CADSceneLoader();

	static RenderNode* Load(const String& name);
private:
};

}

#endif CADSCENE_LOADER_H
