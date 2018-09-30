#ifndef FBX_SCENE_H
#define FBX_SCENE_H

#include <stdint.h>
#include <string>
#include <iostream>
#include <fstream>
#include <aurora/deflate.h>

#include "fbx_node.h"

namespace Fbx {

typedef uint8_t Byte;
typedef uint32_t Word;
typedef uint64_t DoubleWord;

class Scene {
public:
    bool ReadMem(const char* data, size_t size);
    bool ReadFile(const std::string& filename);
private:
    void Finalize(Node& node);
};

}

#endif
