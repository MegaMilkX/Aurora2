#ifndef LOAD_ASSET_H
#define LOAD_ASSET_H

enum ASSET_FILE_EXTENSIONS
{
    TXT,
    JSON,
    GLSL,
    LUA,
    FBX,
    PNG,
    JPG,
    OGG,
    TTF,
    OTF
};

template<typename ASSET, int EXT>
bool LoadAsset(ASSET* asset, const std::string& filename)
{
    std::cout << "Loader not implemented for '" << filename << "'";
    return 0;
}

#endif
