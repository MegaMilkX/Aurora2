#ifndef UTIL_FBX_SCENE_H
#define UTIL_FBX_SCENE_H

#include <component.h>
#include <aurora/media/fbx.h>

inline void LoadSceneFromFbx(SceneObject* scene, const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if(!file.is_open())
        return;
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> buffer((unsigned int)size);
    if(file.read(buffer.data(), size))
    {        
        Au::Media::FBX::Reader fbxReader;
        fbxReader.ReadMemory(buffer.data(), buffer.size());
        fbxReader.DumpFile(filename);
        for(unsigned i = 0; i < fbxReader.ModelCount(); ++i)
        {
            Au::Media::FBX::Model* mdl = fbxReader.GetModel(i);
            SceneObject* o = scene->CreateObject();
            o->Name(mdl->name);
            std::cout << "Created '" << mdl->name << "' object" << std::endl;
        }
    }
}

#endif
