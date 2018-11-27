#ifndef COMPONENT_MODEL_H
#define COMPONENT_MODEL_H

#include "../component.h"
#include "transform.h"
#include <material.h>

#include <mesh.h>

#include <light_omni.h>

#include "../util/gl_helpers.h"
#include "../util/gl_render_state.h"

#include <mutex>

#include <resources/resource/resource_factory.h>
#include <resources/resource/resource_ref.h>

#include <editor/editor_window_pool.h>
#include <editor/editor_data_pick.h>
#include <editor/editor_material.h>
#include <util/editor_gui_helpers.h>

#undef GetObject

inline void write_resource(std::ostream& out, std::shared_ptr<Resource> res) {
    struct resource_header {
        char storage;
        uint64_t size;
    };
    if(res) {
        if(res->Storage() == Resource::LOCAL) {
            std::vector<unsigned char> buf;
            res->Serialize(buf);
            resource_header rh {
                (char)Resource::LOCAL,
                buf.size()
            };
            out.write((char*)&rh, sizeof(rh));
            out.write((char*)buf.data(), buf.size());
        } else if(res->Storage() == Resource::GLOBAL) {
            resource_header rh {
                (char)Resource::GLOBAL,
                res->Name().size()
            };
            out.write((char*)&rh, sizeof(rh));
            out.write(res->Name().c_str(), res->Name().size());
        }
    } else {
        resource_header rh { 0, 0 };
        out.write((char*)&rh, sizeof(rh));
    }
}

template<typename T>
void read_resource(std::istream& in, std::shared_ptr<T>& res) {
    struct resource_header {
        char storage;
        uint64_t size;
    };
    resource_header rh{
        0, 0
    };
    in.read((char*)&rh, sizeof(rh));

    if(rh.size == 0) return;

    if(rh.storage == (char)Resource::LOCAL) {
        std::vector<char> buf;
        buf.resize(rh.size);
        in.read((char*)buf.data(), rh.size);
        DataSourceRef dsmr(new DataSourceMemory(buf.data(), buf.size()));
        res.reset(new T());
        res->Storage(Resource::LOCAL);
        res->Build(dsmr);
    } else if(rh.storage == (char)Resource::GLOBAL) {
        std::string res_name;
        res_name.resize(rh.size);
        in.read((char*)res_name.data(), rh.size);
        res = GlobalResourceFactory().Get<T>(res_name);
    }
}

class Model : public Component
{
public:
    CLONEABLE
    RTTR_ENABLE(Component)
public:
    Model();    
    ~Model();

    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> material;

    void SetMesh(std::string res) {
        std::cout << "Model::SetMesh " << res << std::endl; 
        mesh = GlobalResourceFactory().Get<Mesh>(res); meshName = res; 
    }
    std::string GetMesh() const { return meshName; }
    void SetMaterial(std::string res)
    { material = GlobalResourceFactory().Get<Material>(res); materialName = res; }
    std::string GetMaterial() const { return materialName; }
    
    virtual void OnInit();

    // ====
    virtual bool _write(std::ostream& out, ExportData& exportData) {
        write_resource(out, std::dynamic_pointer_cast<Resource>(mesh));
        write_resource(out, std::dynamic_pointer_cast<Resource>(material));
        return true;
    }
    virtual bool _read(std::istream& in, size_t sz, ImportData& importData) {
        read_resource(in, mesh);
        read_resource(in, material);
        return true;
    }
    virtual bool _editor() {
        ButtonResource("Mesh", mesh, ".geo");
        ButtonResource("Material", material, ".mat");
        if(ImGui::Button("Create material")) {
            material.reset(new Material());
            material->Storage(Resource::LOCAL);
        }
        if(material) {
            if(ImGui::Button("Edit material...")) {
                auto window = Editor::GUI::WindowPool::Create<EditorMaterial>();
                window->SetTarget(material);
            }
        }
        return true;
    }
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
