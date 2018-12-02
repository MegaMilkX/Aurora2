#ifndef GHOST_COLLIDER_H
#define GHOST_COLLIDER_H

#include <component.h>
#include <transform.h>
#include <util/gfxm.h>

#include "physical_object.h"

class GhostCollider : public PhysicalObject {
    RTTR_ENABLE(PhysicalObject)
public:
    virtual void OnInit() {
        ghostObject.reset(new btGhostObject());

        Get<Transform>()->AddTransformCallback([this](){
            gfxm::mat4 m = Get<Transform>()->GetTransform();
            btTransform btMat4;
            btMat4.setFromOpenGLMatrix((btScalar*)&m);
            ghostObject->setWorldTransform(btMat4);
        });
        OnShapeChange();

        ghostObject->setCollisionFlags(
            ghostObject->getCollisionFlags() ^ btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT
        );
    }

    virtual void OnShapeChange() {
        ghostObject->setCollisionShape(shape->GetBtShapePtr());
    }

    btGhostObject* GetBtGhostObject() { return ghostObject.get(); }
    btDiscreteDynamicsWorld* GetBtWorld() { return world; }

    void SetBtWorld(btDiscreteDynamicsWorld* w) { world = w; }

    virtual bool _editor() {
        if(ImGui::BeginCombo("Type", shape_type.get_name().to_string().c_str(), 0)) {
            if(ImGui::Selectable("Box", shape_type == rttr::type::get<BoxCollisionShape>())) { SetShape<BoxCollisionShape>(); }
            if(ImGui::Selectable("Sphere", shape_type == rttr::type::get<SphereCollisionShape>())) { SetShape<SphereCollisionShape>(); }
            if(ImGui::Selectable("Capsule", shape_type == rttr::type::get<CapsuleCollisionShape>())) { SetShape<CapsuleCollisionShape>(); }
/*
            if(ImGui::Selectable("Cylinder", type == CYLINDER)) { type = CYLINDER; Refresh(); }
            
            if(ImGui::Selectable("Cone", type == CONE)) { type = CONE; Refresh(); }
            if(ImGui::Selectable("Plane", type == PLANE)) { type = PLANE; Refresh(); }
            if(ImGui::Selectable("Static plane", type == STATIC_PLANE)) { type = STATIC_PLANE; Refresh(); }
            */
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
    std::shared_ptr<btGhostObject> ghostObject;
    btDiscreteDynamicsWorld* world = 0;
};
STATIC_RUN(GhostCollider)
{
    rttr::registration::class_<GhostCollider>("GhostCollider")
        .constructor<>()(rttr::policy::ctor::as_raw_ptr);
}

#endif
