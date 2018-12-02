#ifndef RIGID_BODY_H
#define RIGID_BODY_H

#include <component.h>
#include <transform.h>
#include <util/gfxm.h>

#include "physical_object.h"

class RigidBody : public PhysicalObject {
    //CLONEABLE
    RTTR_ENABLE(PhysicalObject)
public:
    float mass = 1.0f;

    virtual void OnInit() {
        gfxm::mat4 m = Get<Transform>()->GetTransform();
        btTransform btMat4;
        btMat4.setFromOpenGLMatrix((btScalar*)&m);

        motionState.reset(new btDefaultMotionState(btMat4));
        btVector3 inertia(0, 0, 0);
        shape->GetBtShapePtr()->calculateLocalInertia(mass, inertia);
        rigidBody.reset(
            new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(
                mass,
                motionState.get(),
                shape->GetBtShapePtr(),
                inertia
            ))
        );

        Get<Transform>()->AddTransformCallback([this](){
            if(transformLocked) return;
            gfxm::mat4 m = Get<Transform>()->GetTransform();
            btTransform btMat4;
            btMat4.setFromOpenGLMatrix((btScalar*)&m);
            rigidBody->setWorldTransform(btMat4);
        });
        OnShapeChange();
    }

    void LockTransform() {
        transformLocked = true;
    }
    void UnlockTransform() {
        transformLocked = false;
    }

    btRigidBody* GetBtRigidBody() { return rigidBody.get(); }

    virtual void OnShapeChange() {
        rigidBody->setCollisionShape(shape->GetBtShapePtr());
        btVector3 inertia(0, 0, 0);
        shape->GetBtShapePtr()->calculateLocalInertia(mass, inertia);
        rigidBody->setMassProps(mass, inertia);
        
        rigidBody->activate();
    }

    void UpdateTransform() {
        if(rigidBody->isActive()) {
            LockTransform();
            btTransform trans;
            rigidBody->getMotionState()->getWorldTransform(trans);
            gfxm::mat4 mat4f(1.0f);
            trans.getOpenGLMatrix((btScalar*)&mat4f);
            Get<Transform>()->SetTransform(mat4f);
            UnlockTransform();
        }
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
        if(ImGui::Button("Activate")) {
            rigidBody->activate();
        }
        if(ImGui::DragFloat("Mass", &mass, 0.001f)) {
            btVector3 inertia(0, 0, 0);
            shape->GetBtShapePtr()->calculateLocalInertia(mass, inertia);
            rigidBody->setMassProps(mass, inertia);
        }     
        if(ImGui::BeginCombo("Type", shape_type.get_name().to_string().c_str(), 0)) {
            if(ImGui::Selectable("Box", shape_type == rttr::type::get<BoxCollisionShape>())) { SetShape<BoxCollisionShape>(); }
            if(ImGui::Selectable("Sphere", shape_type == rttr::type::get<SphereCollisionShape>())) { SetShape<SphereCollisionShape>(); }
            if(ImGui::Selectable("Capsule", shape_type == rttr::type::get<CapsuleCollisionShape>())) { SetShape<CapsuleCollisionShape>(); }
/*
            if(ImGui::Selectable("Cylinder", type == CYLINDER)) { type = CYLINDER; Refresh(); }
            
            if(ImGui::Selectable("Cone", type == CONE)) { type = CONE; Refresh(); }
            if(ImGui::Selectable("Plane", type == PLANE)) { type = PLANE; Refresh(); }
            if(ImGui::Selectable("Static plane", type == STATIC_PLANE)) { type = STATIC_PLANE; Refresh(); }
            if(ImGui::Selectable("Concave mesh", type == CONCAVE_MESH)) { type = CONCAVE_MESH; Refresh(); }
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
    bool transformLocked;
    std::shared_ptr<btDefaultMotionState> motionState;
    std::shared_ptr<btRigidBody> rigidBody;
};
STATIC_RUN(RigidBody)
{
    rttr::registration::class_<RigidBody>("RigidBody")
        .constructor<>()(rttr::policy::ctor::as_raw_ptr);
}

#endif
