#ifndef SCENE_RENDERER_H
#define SCENE_RENDERER_H


#include "components/model.h"
#include "components/skin.h"
#include "components/transform.h"
#include "components/bone.h"
#include "components/camera.h"
#include "components/environment.h"
#include <map>
#include <set>
#include "debug_draw.h"
#include "common.h"

#include "data_headers/test.png.h"

#include <resources/resource/resource_factory.h>

struct Renderable
{
    Transform* transform;
    //GLuint vao;
    //int indexCount;
    //std::shared_ptr<Texture2D> texDiffuse;
    Model* model;
    Skin* skin = 0;
};

#include "rendering/gbuffer.h"
#include "rendering/skin_shader_program.h"


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

    glDeleteVertexArrays(1, &vao_handle);
    glDeleteBuffers(1, &vbuf);
}

class SceneRenderer
{
public:
    BasicShaderProgram basicProg;
    SkinShaderProgram skinProg;
    bool Init()
    {
        //_initStaticProgram();
        if(!basicProg.Init()) {
            LOG("Failed to init basic shader program");
            return false;
        }
        if(!skinProg.Init()) {
            LOG("Failed to init skin shader program");
            return false;
        }
        
        _initLightPassProg();
        gBuffer.Init(1280, 720);

        test_texture.reset(new Texture2D());
        test_texture->Build(
            DataSourceRef(
                new DataSourceMemory(
                    (char*)test_png, 
                    sizeof(test_png)
                )
            )
        );

        texture_white_px.reset(new Texture2D());
        unsigned char wht[3] = { 255, 255, 255 };
        texture_white_px->Data(wht, 1, 1, 3);

        texture_black_px.reset(new Texture2D());
        unsigned char blk[3] = { 0, 0, 0};
        texture_black_px->Data(blk, 1, 1, 3);

        return true;
    }

    void SetCamera(Camera* cam) { camera = cam; }
    Camera* GetCamera() { return camera; }

    GBuffer* GetGBuffer() { return &gBuffer; }

    void Render() {
        if(!camera) return;
        gfxm::mat4 projection = camera->Projection();
        gfxm::mat4 view = camera->InverseTransform();
        gfxm::vec3 viewPos = camera->Get<Transform>()->WorldPosition();

        _drawGBuffer(projection, view, viewPos);
        _lightPass(viewPos);

        //DrawTexture2DToScreen(gBuffer.albedo);
        _drawDebugElements(projection, view);
    }

    void _addRenderable(Model* m, SceneObject* so)
    {
        renderables[so].transform = so->Get<Transform>();
        renderables[so].model = m;
    }

    void _onAddComponent(rttr::type type, Component* c, SceneObject* so)
    {
        if(type == rttr::type::get<Environment>()) {
            environment = (Environment*)c;
        }
        else if(type == rttr::type::get<Model>()) {
            Model* m = (Model*)c;            
            _addRenderable(m, so);
        }
        else if(type == rttr::type::get<Skin>()) {
            renderables[so].skin = (Skin*)c;
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
        else if(type == rttr::type::get<Bone>()) {
            bones.insert((Bone*)c);
            for(auto it = bones.begin(); it != bones.end();) {
                SceneObject* parent = (*it)->Object()->Parent();
                if(parent && parent->FindComponent<Bone>()) {
                    bones.erase(it++);
                }
                else {
                    ++it;
                }
            }
        }
    }

    void _onRemoveComponent(rttr::type type, Component* c, SceneObject* so)
    {
        if(type == rttr::type::get<Environment>()) {
            if(environment == (Environment*)c) {
                environment = 0;
            }
        }
        else if(type == rttr::type::get<Model>()) {
            renderables.erase(so);
        }
        else if(type == rttr::type::get<Skin>()) {
            if(renderables.count(so)) {
                renderables[so].skin = 0;
            }
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
        else if(type == rttr::type::get<Bone>()) {
            bones.erase((Bone*)c);
            for(auto it = bones.begin(); it != bones.end();) {
                SceneObject* parent = (*it)->Object()->Parent();
                if(parent && parent->FindComponent<Bone>()) {
                    bones.erase(it++);
                }
                else {
                    ++it;
                }
            }
        }
    }
private:
    void _drawGBuffer(gfxm::mat4& projection, gfxm::mat4& view, gfxm::vec3& viewPos)
    {
        gBuffer.Bind();
        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, Common.frameSize.x, Common.frameSize.y);

        gfxm::vec3 ambientColor(0.5f, 0.5f, 0.5f);
        if(environment) {
            ambientColor = environment->ambientColor;
        }

        glClearColor(ambientColor.x, ambientColor.y, ambientColor.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        //auto& in = staticDrawData;
        BasicShaderProgram* prog = 0;

        for(auto& kv : renderables)
        {
            Renderable& unit = kv.second;
            GLuint vao;
            if(!unit.model->mesh) continue;
            if(unit.model->mesh->vertexCount <= 0) continue;

            if(unit.skin) {
                prog = &skinProg;
                vao = unit.model->mesh->GetSkinVao();
                skinProg.program->Use();
                
                auto& inverse_bind_transforms = unit.skin->GetInverseBindTransforms();
                auto& skin_transforms = unit.skin->Update();
                gfxm::mat4 bindTransform = unit.skin->GetBindTransform();

                GLuint loc = skinProg.program->GetUniform("SkinBindPose");
                glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat*)&bindTransform);

                loc = skinProg.program->GetUniform("BoneInverseBindTransforms[0]");
                glUniformMatrix4fv(
                    loc, 
                    (std::min)((unsigned)100, (unsigned)inverse_bind_transforms.size()), 
                    GL_FALSE, 
                    (GLfloat*)inverse_bind_transforms.data()
                );
                
                loc = skinProg.program->GetUniform("BoneTransforms[0]");
                glUniformMatrix4fv(
                    loc, 
                    (std::min)((unsigned)100, (unsigned)skin_transforms.size()), 
                    GL_FALSE, 
                    (GLfloat*)skin_transforms.data()
                );
            } else {
                prog = &basicProg;
                vao = unit.model->mesh->GetVao();
                basicProg.program->Use();
            }

            glUniformMatrix4fv(prog->uProjection, 1, GL_FALSE, (float*)&projection);
            glUniformMatrix4fv(prog->uView, 1, GL_FALSE, (float*)&view);
            glUniform3f(prog->program->GetUniform("ViewPos"), viewPos.x, viewPos.y, viewPos.z);
            glUniform3f(prog->uAmbientColor, 0.1f, 0.15f, 0.25f);   

            glUniformMatrix4fv(
                prog->uModel, 1, GL_FALSE,
                (float*)&unit.transform->GetTransform()
            );

            if(unit.model->material) {
                glUniform3f(
                    prog->program->GetUniform("Tint"), 
                    unit.model->material->tint.x,
                    unit.model->material->tint.y,
                    unit.model->material->tint.z
                );
                glUniform1f(
                    prog->program->GetUniform("Glossiness"),
                    unit.model->material->glossiness
                );
                glUniform1f(
                    prog->program->GetUniform("Emission"),
                    unit.model->material->emission
                );
                
                glActiveTexture(GL_TEXTURE0);
                if(unit.model->material->diffuseMap) {
                    glBindTexture(GL_TEXTURE_2D, unit.model->material->diffuseMap->GetGlName());
                } else {
                    glBindTexture(GL_TEXTURE_2D, texture_white_px->GetGlName());
                }
                glActiveTexture(GL_TEXTURE1);
                if(unit.model->material->normalMap) {
                    glBindTexture(GL_TEXTURE_2D, unit.model->material->normalMap->GetGlName());
                } else {
                    glBindTexture(GL_TEXTURE_2D, texture_black_px->GetGlName());
                }
                glActiveTexture(GL_TEXTURE2);
                if(unit.model->material->specularMap) {
                    glBindTexture(GL_TEXTURE_2D, unit.model->material->specularMap->GetGlName());
                } else {
                    glBindTexture(GL_TEXTURE_2D, texture_white_px->GetGlName());
                }
            } else {
                glUniform3f(
                    prog->program->GetUniform("Tint"), 
                    1, 1, 1
                );
                glUniform1f(
                    prog->program->GetUniform("Glossiness"),
                    0
                );
                glUniform1f(
                    prog->program->GetUniform("Emission"),
                    0
                );

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, texture_white_px->GetGlName());
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, texture_black_px->GetGlName());
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, texture_white_px->GetGlName());
            }
            
            glBindVertexArray(vao);
            glDrawElements(GL_TRIANGLES, unit.model->mesh->GetIndexCount(), GL_UNSIGNED_INT, (void*)0);
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
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, gBuffer.emission);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        gfxm::vec3 ambientColor(0.1f, 0.1f, 0.1f);

        if(environment) {
            ambientColor = environment->ambientColor;
        }
        
        glUniform3f(lightPassProg->GetUniform("AmbientColor"), ambientColor.x, ambientColor.y, ambientColor.z);
        glUniform3f(lightPassProg->GetUniform("ViewPos"), viewPos.x, viewPos.y, viewPos.z);

        int lightId = 0;
        for(auto& l : lightsDirect)
        {
            auto dir = l->Get<Transform>()->Forward();
            glUniform3f(
                lightPassProg->GetUniform("LightDirect[" + std::to_string(lightId) + "]"), 
                dir.x, dir.y, dir.z
            );
            auto col = l->Color() * l->Intensity();
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
        glUniform1i(lightPassProg->GetUniform("inEmission"), 4);
    }

    void _drawDebugElements(const gfxm::mat4& p, const gfxm::mat4& v)
    {
        //float color[3] = { 0.3f, 0.3f, 0.3f };
        //dd::xzSquareGrid(-10.0f, 10.0f, 0.0f, 0.5f, color);
/*
        for(auto t : transforms)
        {
            gfxm::mat4 m = t->GetTransform();
            gfxm::vec4 pos = m[3];
            gfxm::mat3 o = gfxm::to_orient_mat3(m);
            m = o;
            m[3] = pos;
            dd::axisTriad((float*)&m, 0.01f, 0.1f, 0, false);
        }
*/

        auto draw_line_between_bones = [](Bone* a, Bone* b) {
            float bone_color[3] = { 1.0f, 0.6833f, 0.0f };
            dd::line(
                (float*)&a->Get<Transform>()->WorldPosition(),
                (float*)&b->Get<Transform>()->WorldPosition(),
                bone_color, 0, false
            );
        };

        std::function<void(Bone*)> draw_lines_to_children_bones;
        draw_lines_to_children_bones = [&draw_line_between_bones, &draw_lines_to_children_bones](Bone* b) {
            SceneObject* so = b->Object();
            for(unsigned i = 0; i < so->ChildCount(); ++i) {
                Bone* child_b = so->GetChild(i)->FindComponent<Bone>();
                if(child_b) {
                    draw_line_between_bones(b, child_b);
                    draw_lines_to_children_bones(child_b);
                }
            }
        };

        for(auto b : bones) {
            draw_lines_to_children_bones(b);
        }

        DebugDraw::Draw(p * v);
    }
    Camera* camera;
    std::map<SceneObject*, Renderable> renderables;
    std::set<LightDirect*> lightsDirect;
    std::set<LightOmni*> lightsOmni;
    std::set<Transform*> transforms;
    std::set<Bone*> bones;

    Environment* environment = 0;

    gl::ShaderProgram* lightPassProg;

    GBuffer gBuffer;

    //
    std::shared_ptr<Texture2D> test_texture;
    std::shared_ptr<Texture2D> texture_white_px;
    std::shared_ptr<Texture2D> texture_black_px;
};

#endif
