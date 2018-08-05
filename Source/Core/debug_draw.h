#ifndef DEBUG_DRAW_H
#define DEBUG_DRAW_H

#include "../lib/dd_render_intarface_core_gl.h"

class DebugDraw
{
public:
    static void Init()
    {
        ddRenderInterface = new DDRenderInterfaceCoreGL();
        dd::initialize(ddRenderInterface);
    }

    static void Draw(const gfxm::mat4& vp)
    {
        ddRenderInterface->mvpMatrix = vp;
        dd::flush();
    }

    static void Cleanup()
    {
        dd::shutdown();
    }
private:
    static DDRenderInterfaceCoreGL* ddRenderInterface;
};

#endif
