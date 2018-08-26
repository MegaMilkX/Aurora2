#ifndef FBX_LIGHT_H
#define FBX_LIGHT_H

#include <stdint.h>
#include <string>
#include "fbx_object.h"

class FbxLight : FbxObject
{
public:
    static char* Type() { return "Light"; }
};

#endif
