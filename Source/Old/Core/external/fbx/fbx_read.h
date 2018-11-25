#ifndef FBX_READ_H
#define FBX_READ_H

#include "fbx_scene.h"
#include "fbx_node.h"

typedef uint8_t FbxByte;
typedef uint32_t FbxWord;
typedef uint64_t FbxDoubleWord;

bool FbxReadMem(FbxScene& out_scene, const char* data, size_t size);
bool FbxReadFile(FbxScene& out_scene, const std::string& filename);

#endif
