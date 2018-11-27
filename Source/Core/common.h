#ifndef ENGINE_COMMON_H
#define ENGINE_COMMON_H

#include <util/gfxm.h>

struct Common_t
{
    gfxm::ivec2 frameSize;
    float frameDelta;
};

extern Common_t Common;

#endif
