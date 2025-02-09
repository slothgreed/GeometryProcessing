#include "CADSceneLoader.h"
#include <fileformats/cadscenefile.h>

namespace KI
{
CADSceneLoader::CADSceneLoader()
{
}

CADSceneLoader::~CADSceneLoader()
{

}

RenderNode* CADSceneLoader::Load(const String& fileName)
{
    return nullptr;
    CSFile* csf;
    CSFileMemoryPTR mem = CSFileMemory_new();
    //CSFileMemoryPTR mem = new CSFileMemory_s;
    /*if (CSFile_loadExt(&csf, filename, mem) != CADSCENEFILE_NOERROR || !(csf->fileFlags & CADSCENEFILE_FLAG_UNIQUENODES)) {
        CSFileMemory_delete(mem);
        return false;
    }
    */
}
}