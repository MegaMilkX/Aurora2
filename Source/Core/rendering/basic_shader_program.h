#ifndef BASIC_SHADER_PROGRAM_H
#define BASIC_SHADER_PROGRAM_H

#include <memory>
#include <util/gl_render_state.h>
#include "../../general/util.h"
#include <util/split.h>

class BasicShaderProgram {
public:
    virtual ~BasicShaderProgram() {}

    bool Init() {
        LOG("Initializing shader program...");
        program = std::shared_ptr<gl::ShaderProgram>(new gl::ShaderProgram());
        gl::ShaderProgram& p = *program.get();
        gl::Shader vs;
        gl::Shader fs;
        vs.Init(GL_VERTEX_SHADER);
        vs.Source(VertexShaderSource());
        vs.Compile();

        fs.Init(GL_FRAGMENT_SHADER);
        fs.Source(FragmentShaderSource());
        fs.Compile();

        p.AttachShader(&vs);
        p.AttachShader(&fs);
        p.BindAttrib(0, "Position");
        p.BindAttrib(1, "UV");
        p.BindAttrib(2, "Normal");
        BindAdditionalAttribs(p, 3);
        p.BindFragData(0, "outAlbedo");
        p.BindFragData(1, "outPosition");
        p.BindFragData(2, "outNormal");
        p.BindFragData(3, "outSpecular");
        p.Link();

        p.Use();
        glUniform1i(p.GetUniform("DiffuseTexture"), 0);
        glUniform1i(p.GetUniform("NormalTexture"), 1);
        glUniform1i(p.GetUniform("SpecularTexture"), 2);

        uAmbientColor = p.GetUniform("AmbientColor");
        uModel = p.GetUniform("MatrixModel");
        uProjection = p.GetUniform("MatrixProjection");
        uView = p.GetUniform("MatrixView");

        LOG("Done");
        return true;
    }

    std::shared_ptr<gl::ShaderProgram> program;
    GLuint uProjection;
    GLuint uView;
    GLuint uModel;
    GLuint uAmbientColor;

private:
    virtual const char* VertexShaderSource() {
        return 
            #include "shaders/gbuffer_vs.glsl"
            ;
    }
    virtual const char* FragmentShaderSource() {
        return 
             #include "shaders/gbuffer_fs.glsl"
             ;
    }
    virtual void BindAdditionalAttribs(gl::ShaderProgram& p, int nextAttribIndex) {

    }
};

#endif
