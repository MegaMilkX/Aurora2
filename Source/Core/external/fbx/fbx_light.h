#ifndef FBX_LIGHT_H
#define FBX_LIGHT_H

#include <stdint.h>
#include <string>
#include "fbx_object.h"

class FbxLight : public FbxObject
{
public:
    static const char* Type() { return "Light"; }
};

#endif
