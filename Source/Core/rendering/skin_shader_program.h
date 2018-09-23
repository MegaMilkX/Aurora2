#ifndef SKIN_SHADER_PROGRAM_H
#define SKIN_SHADER_PROGRAM_H

#include "basic_shader_program.h"

class SkinShaderProgram : public BasicShaderProgram {
public:
private:
    virtual const char* VertexShaderSource() {
        return 
            #include "shaders/skin_vs.glsl"
            ;
    }

    virtual void BindAdditionalAttribs(gl::ShaderProgram& p, int nextAttribIndex) {
        p.BindAttrib(nextAttribIndex, "BoneIndex4");
        p.BindAttrib(nextAttribIndex + 1, "BoneWeight4");
    }
};

#endif
