#ifndef COMPONENT_SKIN_MESH_H
#define COMPONENT_SKIN_MESH_H

#include "../scene_object.h"
#include <component.h>
#include "transform.h"
#include "renderer.h"
#include <material.h>

#include <mesh.h>

#undef GetObject

class SkinMesh : public SceneObject::Component
{
public:
    SkinMesh()
    : meshData(0), material(0)
    {}
    void SetMesh(const std::string& name)
    {
        SetMesh(asset<Mesh>::get(name));
    }
    void SetMesh(Mesh* md) { meshData = md; }
    void SetMaterial(const std::string& name)
    {
        SetMaterial(asset<Material>::get(name));
    }
    void SetMaterial(Material* mat) { material = mat; }
    
    Mesh* GetMesh() { return meshData; }
    Material* GetMaterial() { return material; }
    
    void OnInit()
    {
        Renderer* renderer = GetObject()->Root()->GetComponent<Renderer>();
    }
private:
    asset<Mesh> meshData;
    asset<Material> material;
};
COMPONENT(SkinMesh)

#endif
