#ifndef COMPONENT_MODEL_H
#define COMPONENT_MODEL_H

#include "../scene_object.h"
#include "transform.h"
#include <material.h>

#include <mesh.h>

#include <light_omni.h>

#include "../util/gl_helpers.h"
#include "../util/gl_render_state.h"

#include <mutex>

#include <resources/resource/resource_ref.h>
#include <resources/resource/resource_factory.h>

#undef GetObject

class Model : public SceneObject::Component
{
    CLONEABLE(Model)
    RTTR_ENABLE(SceneObject::Component)
public:
    Model();    
    ~Model();

    ResourceRef mesh;
    ResourceRef material;

    resource<gl::ShaderProgram> program;

    void SetMesh(std::string res) {
        std::cout << "Model::SetMesh " << res << std::endl; 
        mesh.Set(GlobalResourceFactory().Get<Mesh>(res)); meshName = res; 
    }
    std::string GetMesh() const { return meshName; }
    void SetMaterial(std::string res)
    { material.Set(GlobalResourceFactory().Get<Material>(res)); materialName = res; }
    std::string GetMaterial() const { return materialName; }
    
    virtual void OnInit();
protected:    
    bool dirty;

    Transform* transform;

    std::string materialName;
    std::string meshName;    
    std::string subMeshName;
};
STATIC_RUN(Model)
{
    rttr::registration::class_<Model>("Model")
        .constructor<>()(rttr::policy::ctor::as_raw_ptr)
        .property(
            "mesh", &Model::mesh
        )
        .property(
            "material", &Model::material
        );
}

#endif
