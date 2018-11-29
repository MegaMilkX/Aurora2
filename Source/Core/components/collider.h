#ifndef COLLIDER_H
#define COLLIDER_H

#include <component.h>
#include <transform.h>
#include <util/gfxm.h>

#include "physical_object.h"

class Collider : public PhysicalObject {
    //CLONEABLE
    RTTR_ENABLE(PhysicalObject)
public:

    virtual void OnInit() {
        collisionObject.reset(new btCollisionObject());

        Get<Transform>()->AddTransformCallback([this](){
            gfxm::mat4 m = Get<Transform>()->GetTransform();
            btTransform btMat4;
            btMat4.setFromOpenGLMatrix((btScalar*)&m);
            collisionObject->setWorldTransform(btMat4);
        });
        OnShapeChange();
    }

    virtual void OnShapeChange() {
        collisionObject->setCollisionShape(shape->GetBtShapePtr());
    }

    btCollisionObject* GetBtCollisionObject() { return collisionObject.get(); }

    void _enableDebugWireframe() {
        collisionObject->setCollisionFlags(
            collisionObject->getCollisionFlags() ^ btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT
        );
    }
    void _disableDebugWireframe() {
        collisionObject->setCollisionFlags(
            collisionObject->getCollisionFlags() | btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT
        );
        
    }

    virtual bool _write(std::ostream& out, ExportData& exportData) {
        //out.write((char*)&color, sizeof(color));
        //out.write((char*)&intensity, sizeof(intensity));
        return true;
    }
    virtual bool _read(std::istream& in, size_t sz, ImportData& importData) {
        //if(sz != sizeof(color) + sizeof(intensity)) 
        //    return false;
        //in.read((char*)&color, sizeof(color));
        //in.read((char*)&intensity, sizeof(intensity));
        return true;
    }
    virtual bool _editor() {        
        if(ImGui::BeginCombo("Type", shape_type.get_name().to_string().c_str(), 0)) {
            if(ImGui::Selectable("Box", shape_type == rttr::type::get<BoxCollisionShape>())) { SetShape<BoxCollisionShape>(); }
            if(ImGui::Selectable("Sphere", shape_type == rttr::type::get<SphereCollisionShape>())) { SetShape<SphereCollisionShape>(); }
/*
            if(ImGui::Selectable("Cylinder", type == CYLINDER)) { type = CYLINDER; Refresh(); }
            if(ImGui::Selectable("Capsule", type == CAPSULE)) { type = CAPSULE; Refresh(); }
            if(ImGui::Selectable("Cone", type == CONE)) { type = CONE; Refresh(); }
            if(ImGui::Selectable("Plane", type == PLANE)) { type = PLANE; Refresh(); }
            if(ImGui::Selectable("Static plane", type == STATIC_PLANE)) { type = STATIC_PLANE; Refresh(); }
            */
            if(ImGui::Selectable("Triangle mesh", shape_type == rttr::type::get<TriangleMeshCollisionShape>())) 
            { SetShape<TriangleMeshCollisionShape>(); }
            /*
            if(ImGui::Selectable("Convex mesh", type == CONVEX_MESH)) { type = CONVEX_MESH; Refresh(); }
            */
            ImGui::EndCombo();
        }

        if(shape) {
            shape->_editor(this);
        }

        return true;
    }
private:
    std::shared_ptr<btCollisionObject> collisionObject;
};
STATIC_RUN(Collider)
{
    rttr::registration::class_<Collider>("Collider")
        .constructor<>()(rttr::policy::ctor::as_raw_ptr);
}

#endif
