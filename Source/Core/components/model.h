#ifndef COMPONENT_MODEL_H
#define COMPONENT_MODEL_H

#include "../scene_object.h"
#include "transform.h"
#include "renderer.h"
#include <material.h>

#include <mesh.h>

#include <light_omni.h>

#include "../util/gl_helpers.h"
#include "../util/gl_render_state.h"

#include <mutex>

#include <resource_ref.h>

#undef GetObject
/*
struct RenderUnitSolid
{    
    GLuint vao;
    int indexCount;
    asset<Texture2D> texDiffuse;

    int vertexSize;
    Transform* transform;
};

struct SolidDrawData
{
    resource<gl::ShaderProgram> program;
    GLuint uniProjection;
    GLuint uniView;
    GLuint uniModel;
    GLuint uniAmbientColor;
    std::vector<RenderUnitSolid> units;
};

struct SolidDrawStats
{
    int drawCalls;
};

inline void fg_SolidRebuild(const FrameCommon& frame, SolidDrawData& out);

inline void fg_SolidDraw(
    const FrameCommon& frame, 
    const SolidDrawData& in)
{
    in.program->Use();
    
    glUniformMatrix4fv(
        in.uniProjection, 1, GL_FALSE,
        (float*)&frame.projection
    );
    glUniformMatrix4fv(
        in.uniView, 1, GL_FALSE,
        (float*)&frame.view
    );
    glUniform3f(
        in.program->GetUniform("ViewPos"),
        frame.viewPos.x,
        frame.viewPos.y,
        frame.viewPos.z
    );
    
    glUniform3f(
        in.uniAmbientColor,
        0.1f, 0.15f, 0.25f
    );

    std::vector<LightDirect*> lds = frame.scene->FindAllOf<LightDirect>();
    for(unsigned i = 0; i < lds.size(); ++i)
    {
        auto l = lds[i];
        glUniform3f(
            in.program->GetUniform("LightDirect[" + std::to_string(i) + "]"), 
            l->Direction().x,
            l->Direction().y,
            l->Direction().z
        );
        glUniform3f(
            in.program->GetUniform("LightDirectRGB[" + std::to_string(i) + "]"), 
            l->Color().x,
            l->Color().y,
            l->Color().z
        );
    }
    std::vector<LightOmni*> los = frame.scene->FindAllOf<LightOmni>();
    for(unsigned i = 0; i < los.size(); ++i)
    {
        auto l = los[i];
        glUniform3f(
            in.program->GetUniform("LightOmniPos[" + std::to_string(i) + "]"), 
            l->Get<Transform>()->WorldPosition().x,
            l->Get<Transform>()->WorldPosition().y,
            l->Get<Transform>()->WorldPosition().z
        );
        glUniform3f(
            in.program->GetUniform("LightOmniRGB[" + std::to_string(i) + "]"), 
            l->Color().x,
            l->Color().y,
            l->Color().z
        );
    }
    
    for(const RenderUnitSolid& unit : in.units)
    {
        glUniformMatrix4fv(
            in.uniModel, 1, GL_FALSE,
            (float*)&unit.transform->GetTransform()
        );
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, unit.texDiffuse->GetGlName());
        
        glBindVertexArray(unit.vao);
        glDrawElements(GL_TRIANGLES, unit.indexCount, GL_UNSIGNED_INT, (void*)0);
    }
}
*/
class Model : public SceneObject::Component
{
    CLONEABLE(Model)
    RTTR_ENABLE(SceneObject::Component)
friend Renderer;
public:
    Model();    
    ~Model();

    ResourceRef mesh;
    ResourceRef material;

    resource<gl::ShaderProgram> program;

    void SetMesh(std::string res)
    { mesh.Set(res); meshName = res; }
    std::string GetMesh() const { return meshName; }
    void SetMaterial(std::string res)
    { material.Set(res); materialName = res; }
    std::string GetMaterial() const { return materialName; }
    
    virtual void OnInit();
protected:    
    bool dirty;

    Transform* transform;
    Renderer* renderer;

    std::string materialName;
    std::string meshName;    
    std::string subMeshName;
};
STATIC_RUN(Model)
{
    rttr::registration::class_<Model>("Model")
        .constructor<>()(rttr::policy::ctor::as_raw_ptr)
        .property(
            "mesh",
            &Model::GetMesh,
            &Model::SetMesh
        )
        .property(
            "material",
            &Model::GetMaterial,
            &Model::SetMaterial
        );
}
/*
void fg_SolidRebuild(const FrameCommon& frame, SolidDrawData& out)
{
    out.units.clear();

    Renderer* renderer = frame.scene->GetComponent<Renderer>();
    std::vector<Model*> meshes = frame.scene->FindAllOf<Model>();
    for(Model* model : meshes)
    {
        if(!model->program.equals(out.program))
            continue;
        if(!model->mesh)
            continue;
        
        RenderUnitSolid unit;
        unit.transform = model->GetComponent<Transform>();
        
        unit.vao = model->mesh->GetVao({
            { "Position", 3, GL_FLOAT, GL_FALSE },
            { "UV", 2, GL_FLOAT, GL_FALSE },
            { "Normal", 3, GL_FLOAT, GL_FALSE }
        });
        unit.indexCount = model->mesh->GetIndexCount();
        
        Texture2D* tex = 
            asset<Texture2D>::get(model->material->GetString("Diffuse"));
        unit.texDiffuse.set(model->material->GetString("Diffuse"));
        
        out.units.push_back(unit);
    }
    
    std::cout << "Created " << out.units.size() << " render units" << std::endl;
}
*/
#endif
