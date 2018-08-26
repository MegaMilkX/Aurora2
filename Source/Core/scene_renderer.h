#ifndef SCENE_RENDERER_H
#define SCENE_RENDERER_H

#include <aurora/gfx.h>
#include "components/model.h"
#include "components/transform.h"
#include "components/camera.h"
#include <map>
#include <set>
#include "debug_draw.h"
#include "common.h"

#include "data_headers/test_texture.png.h"

struct Renderable
{
    Transform* transform;
    GLuint vao;
    int indexCount;
    asset<Texture2D> texDiffuse;
};

struct StaticDrawData
{
    resource<gl::ShaderProgram> program;
    GLuint uProjection;
    GLuint uView;
    GLuint uModel;
    GLuint uAmbientColor;
};

class GBuffer
{
public:
    GBuffer()
    : fbo(0), albedo(0), position(0), normal(0), specular(0)
    {}
    void Init(unsigned width, unsigned height)
    {
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        ResizeBuffers(width, height);  
    }
    void Cleanup()
    {
        glDeleteFramebuffers(1, &fbo);
    }

    void ResizeBuffers(unsigned width, unsigned height)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        _createBuffer(albedo, GL_RGB, GL_UNSIGNED_BYTE, width, height, 0);
        _createBuffer(position, GL_RGB16F, GL_FLOAT, width, height, 1);
        _createBuffer(normal, GL_RGB16F, GL_FLOAT, width, height, 2);
        _createBuffer(specular, GL_RED, GL_UNSIGNED_BYTE, width, height, 3);

        if(depth) glDeleteTextures(1, &depth);
        glGenTextures(1, &depth);
        glBindTexture(GL_TEXTURE_2D, depth);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
            (GLsizei)width, (GLsizei)height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth, 0);

        GLenum a[4] = { 
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2,
            GL_COLOR_ATTACHMENT3
        };
        glDrawBuffers(4, a);

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            LOG_ERR("G-Buffer is incomplete!");
        }
    }

    void Bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    }
    void Unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void _createBuffer(GLuint& t, GLint internalFormat, GLenum type, unsigned width, unsigned height, int attachment)
    {
        if(t) glDeleteTextures(1, &t);
        glGenTextures(1, &t);
        glBindTexture(GL_TEXTURE_2D, t);
        glTexImage2D(
            GL_TEXTURE_2D, 0, internalFormat, 
            (GLsizei)width, (GLsizei)height, 0, GL_RGB, type, 0
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, t, 0);
    }
    GLuint fbo;
    GLuint albedo;
    GLuint position;
    GLuint normal;
    GLuint specular;
    GLuint depth;
};


inline gl::ShaderProgram* CreateScreenQuadShader()
{
    gl::ShaderProgram* screenQuadShader;
    const char* screen_quad_vs = R"(
    #version 450
    in vec3 Vertex;
    in vec2 UV;
    out vec2 UVFrag;
    void main()
    {
        UVFrag = UV;
        gl_Position = vec4(Vertex, 1.0);
    })";

    const char* screen_quad_fs = R"(
    #version 450
    in vec2 UVFrag;
    out vec4 fragOut;
    uniform sampler2D tex;
    void main()
    {
        fragOut = texture(tex, UVFrag);
    })"; 
    screenQuadShader = new gl::ShaderProgram();
    gl::Shader vs;
    gl::Shader fs;
    vs.Init(GL_VERTEX_SHADER);
    vs.Source(screen_quad_vs);
    vs.Compile();

    fs.Init(GL_FRAGMENT_SHADER);
    fs.Source(screen_quad_fs);
    fs.Compile();

    screenQuadShader->AttachShader(&vs);
    screenQuadShader->AttachShader(&fs);
    screenQuadShader->BindAttrib(0, "Vertex");
    screenQuadShader->BindAttrib(1, "UV");
    screenQuadShader->BindFragData(0, "fragOut");
    screenQuadShader->Link();

    screenQuadShader->Use();
    glUniform1i(screenQuadShader->GetUniform("tex"), 0);
    return screenQuadShader;
}

void DrawQuad();
inline void DrawTexture2DToScreen(GLuint texture)
{
    static gl::ShaderProgram* screenQuadShader = CreateScreenQuadShader();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, 1280, 720);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    screenQuadShader->Use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    DrawQuad();
}

inline void DrawQuad()
{
    std::vector<float> vertices = {
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f
    };

    GLuint vao_handle = 0;
    GLuint vbuf;
    glGenBuffers(1, &vbuf);

    glGenVertexArrays(1, &vao_handle);
    glBindVertexArray(vao_handle);
    glBindBuffer(GL_ARRAY_BUFFER, vbuf);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, 
        sizeof(float) * 5, 0
    );
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE, 
        sizeof(float) * 5, (void*)(sizeof(float) * 3)
    );

    glBindBuffer(GL_ARRAY_BUFFER, vbuf);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), (void*)vertices.data(), GL_STREAM_DRAW);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDeleteBuffers(1, &vbuf);
}

class SceneRenderer
{
public:
    bool Init()
    {
        _initStaticProgram();
        _initLightPassProg();
        gBuffer.Init(1280, 720);

        test_texture.reset(new Texture2D());
        ResourceRawMemory raw((char*)test_texture_png, sizeof(test_texture_png));
        test_texture->Build(&raw);

        return true;
    }

    void SetCamera(Camera* cam) { camera = cam; }
    Camera* GetCamera() { return camera; }

    GBuffer* GetGBuffer() { return &gBuffer; }

    void Render() {
        if(!camera) return;
        gfxm::mat4 projection = camera->Projection();
        gfxm::mat4 view = camera->InverseTransform();
        gfxm::vec3 viewPos = camera->Get<Transform>()->Position();

        _drawGBuffer(projection, view, viewPos);
        _lightPass(viewPos);

        //DrawTexture2DToScreen(gBuffer.albedo);
        _drawDebugElements(projection, view);
    }

    void _addRenderable(Model* m, SceneObject* so)
    {
        Mesh* mesh = m->mesh.Get<Mesh>();
        if(!mesh) {
            LOG("No mesh specified");
            return;
        }
        Material* mat = m->material.Get<Material>();
        Texture2D* tex = 0;
        if(mat) tex = ResourceRef(mat->GetString("Diffuse")).Get<Texture2D>();
        if(!tex) tex = test_texture.get();

        renderables[so] = Renderable{
            so->Get<Transform>(),
            m->mesh.Get<Mesh>()->GetVao({
                { "Position", 3, GL_FLOAT, GL_FALSE },
                { "UV", 2, GL_FLOAT, GL_FALSE },
                { "Normal", 3, GL_FLOAT, GL_FALSE }
            }),
            (int)m->mesh.Get<Mesh>()->GetIndexCount(),
            tex
        };

        LOG("Renderable added " << so);
    }

    void _onAddComponent(rttr::type type, SceneObject::Component* c, SceneObject* so)
    {
        if(type == rttr::type::get<Model>()) {
            Model* m = (Model*)c;
            
            m->mesh.AddChangeCallback(
                [this, so, m](ResourceRef* ref){
                    _addRenderable(m, so);
                }
            );
            m->material.AddChangeCallback(
                [this, so, m](ResourceRef* ref){
                    _addRenderable(m, so);
                }
            );

            _addRenderable(m, so);
        }
        else if(type == rttr::type::get<Camera>()) {
            SetCamera((Camera*)c);
        }
        else if(type == rttr::type::get<LightDirect>()) {
            lightsDirect.insert((LightDirect*)c);
        }
        else if(type == rttr::type::get<LightOmni>()) {
            lightsOmni.insert((LightOmni*)c);
        }
        else if(type == rttr::type::get<Transform>()) {
            transforms.insert((Transform*)c);
        }
    }

    void _onRemoveComponent(rttr::type type, SceneObject::Component* c, SceneObject* so)
    {
        if(type == rttr::type::get<Model>()) {
            renderables.erase(so);
            LOG("Renderable removed " << so);
        }
        else if(type == rttr::type::get<Camera>()) {
            if(camera == (Camera*)c) SetCamera(0);
        }
        else if(type == rttr::type::get<LightDirect>()) {
            lightsDirect.erase((LightDirect*)c);
        }
        else if(type == rttr::type::get<LightOmni>()) {
            lightsOmni.erase((LightOmni*)c);
        }
        else if(type == rttr::type::get<Transform>()) {
            transforms.erase((Transform*)c);
        }
    }
private:
    void _drawGBuffer(gfxm::mat4& projection, gfxm::mat4& view, gfxm::vec3& viewPos)
    {
        gBuffer.Bind();
        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, Common.frameSize.x, Common.frameSize.y);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        

        // TODO: Render objects?
        
        auto& in = staticDrawData;
        in.program->Use();
        glUniformMatrix4fv(in.uProjection, 1, GL_FALSE, (float*)&projection);
        glUniformMatrix4fv(in.uView, 1, GL_FALSE, (float*)&view);
        glUniform3f(in.program->GetUniform("ViewPos"), viewPos.x, viewPos.y, viewPos.z);
        glUniform3f(in.uAmbientColor, 0.1f, 0.15f, 0.25f);   

        for(auto& kv : renderables)
        {
            Renderable& unit = kv.second;
            glUniformMatrix4fv(
                in.uModel, 1, GL_FALSE,
                (float*)&unit.transform->GetTransform()
            );
            
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, unit.texDiffuse->GetGlName());

            glBindVertexArray(unit.vao);
            glDrawElements(GL_TRIANGLES, unit.indexCount, GL_UNSIGNED_INT, (void*)0);
        }
        // ==========
    }

    void _lightPass(gfxm::vec3& viewPos)
    {
        lightPassProg->Use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gBuffer.albedo);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gBuffer.position);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gBuffer.normal);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, gBuffer.specular);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUniform3f(lightPassProg->GetUniform("ViewPos"), viewPos.x, viewPos.y, viewPos.z);

        int lightId = 0;
        for(auto& l : lightsDirect)
        {
            auto dir = l->Direction();
            glUniform3f(
                lightPassProg->GetUniform("LightDirect[" + std::to_string(lightId) + "]"), 
                dir.x, dir.y, dir.z
            );
            auto col = l->Color();
            glUniform3f(
                lightPassProg->GetUniform("LightDirectRGB[" + std::to_string(lightId) + "]"), 
                col.x, col.y, col.z
            );
            ++lightId;
        }

        lightId = 0;
        for(auto l : lightsOmni)
        {
            auto pos = l->Get<Transform>()->WorldPosition();
            auto col = l->Color();
            auto intensity = l->Intensity();
            glUniform3f(
                lightPassProg->GetUniform(MKSTR("LightOmniPos[" << lightId << "]")),
                pos.x, pos.y, pos.z
            );
            col = col * intensity;
            glUniform3f(
                lightPassProg->GetUniform(MKSTR("LightOmniRGB[" << lightId << "]")),
                col.x, col.y, col.z
            );
            ++lightId;
        }

        DrawQuad();
    }

    void _initStaticProgram()
    {
        LOG("Initializing static shader program...");
        staticDrawData.program = resource<gl::ShaderProgram>::get("solid_shader");
        resource<gl::ShaderProgram> p = staticDrawData.program;
        gl::Shader vs;
        gl::Shader fs;
        vs.Init(GL_VERTEX_SHADER);
        vs.Source(
            #include "shaders/gbuffer_vs.glsl"
        );
        vs.Compile();

        fs.Init(GL_FRAGMENT_SHADER);
        fs.Source(
            #include "shaders/gbuffer_fs.glsl"
        );
        fs.Compile();

        p->AttachShader(&vs);
        p->AttachShader(&fs);
        p->BindAttrib(0, "Position");
        p->BindAttrib(1, "UV");
        p->BindAttrib(2, "Normal");
        p->BindFragData(0, "outAlbedo");
        p->BindFragData(1, "outPosition");
        p->BindFragData(2, "outNormal");
        p->BindFragData(3, "outSpecular");
        p->Link();

        p->Use();
        glUniform1i(p->GetUniform("DiffuseTexture"), 0);

        staticDrawData.program = p;
        staticDrawData.uAmbientColor = p->GetUniform("AmbientColor");
        staticDrawData.uModel = p->GetUniform("MatrixModel");
        staticDrawData.uProjection = p->GetUniform("MatrixProjection");
        staticDrawData.uView = p->GetUniform("MatrixView");

        LOG("Done");
    }

    void _initLightPassProg()
    {
        lightPassProg = new gl::ShaderProgram();
        gl::Shader vs;
        gl::Shader fs;
        vs.Init(GL_VERTEX_SHADER);
        vs.Source(
            #include "shaders/textured_fullscreen_quad.glsl"
        );
        vs.Compile();

        fs.Init(GL_FRAGMENT_SHADER);
        fs.Source(
            #include "shaders/light_pass.glsl"
        );
        fs.Compile();

        lightPassProg->AttachShader(&vs);
        lightPassProg->AttachShader(&fs);
        lightPassProg->BindAttrib(0, "Vertex");
        lightPassProg->BindAttrib(1, "UV");
        lightPassProg->BindFragData(0, "fragOut");
        lightPassProg->Link();

        lightPassProg->Use();
        glUniform1i(lightPassProg->GetUniform("inAlbedo"), 0);
        glUniform1i(lightPassProg->GetUniform("inPosition"), 1);
        glUniform1i(lightPassProg->GetUniform("inNormal"), 2);
        glUniform1i(lightPassProg->GetUniform("inSpecular"), 3);
    }

    void _drawDebugElements(const gfxm::mat4& p, const gfxm::mat4& v)
    {
        float color[3] = { 0.3f, 0.3f, 0.3f };
        dd::xzSquareGrid(-10.0f, 10.0f, 0.0f, 0.5f, color);

        for(auto t : transforms)
        {
            gfxm::mat4 m = t->GetTransform();
            gfxm::vec4 pos = m[3];
            gfxm::mat3 o = gfxm::to_orient_mat3(m);
            m = o;
            m[3] = pos;
            dd::axisTriad((float*)&m, 0.01f, 0.1f, 0, false);
        }

        DebugDraw::Draw(p * v);
    }
    Camera* camera;
    std::map<SceneObject*, Renderable> renderables;
    std::set<LightDirect*> lightsDirect;
    std::set<LightOmni*> lightsOmni;
    std::set<Transform*> transforms;

    StaticDrawData staticDrawData;
    gl::ShaderProgram* lightPassProg;

    GBuffer gBuffer;

    //
    std::shared_ptr<Texture2D> test_texture;
};

#endif
