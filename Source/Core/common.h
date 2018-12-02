#ifndef ENGINE_COMMON_H
#define ENGINE_COMMON_H

#include <util/gfxm.h>

struct Common_t
{
    enum FRAME_STATE {
        BEGIN,
        PHYSICS,
        GENERAL,
        RENDERING,
        END
    };

    gfxm::ivec2 frameSize;
    float frameDelta;
    void* camera;
};

extern Common_t Common;

#endif
