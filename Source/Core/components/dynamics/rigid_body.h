#ifndef COM_RIGID_BODY_H
#define COM_RIGID_BODY_H

#include <util/gfxm.h>
#include "../../scene_object.h"

#include "dynamics.h"

#include <transform.h>

class RigidBody : public SceneObject::Component
{
friend Dynamics;
public:
    RigidBody(){}
    ~RigidBody(){}
    
    void SetLinearVelocity(const gfxm::vec3& velo)
    {
        SetLinearVelocity(velo.x, velo.y, velo.z);
    }
    
    void SetLinearVelocity(float x, float y, float z)
    {
        rigidBody->activate(true);
        rigidBody->setLinearVelocity(btVector3(x, y, z));
    }
    
    void SetAngularFactor(const gfxm::vec3& vec)
    {
        rigidBody->setAngularFactor(btVector3(vec.x, vec.y, vec.z));
    }
    
    void SetLinearFactor(const gfxm::vec3& vec)
    {
        rigidBody->setLinearFactor(btVector3(vec.x, vec.y, vec.z));
    }
    
    void LookAt(const gfxm::vec3& target, const gfxm::vec3& up)
    {        
        Transform* trans = GetObject()->GetComponent<Transform>();
        trans->LookAt(target, up);
        UpdateFromTransform();
    }
    
    void UpdateFromTransform()
    {
        btTransform trans;
        trans.setFromOpenGLMatrix((btScalar*)&GetObject()->GetComponent<Transform>()->GetTransform());
        rigidBody->setWorldTransform(trans);
    }
    
    void Update()
    {
        btTransform trans;
        rigidBody->getMotionState()->getWorldTransform(trans);
        Transform* t = GetObject()->GetComponent<Transform>();
        
        gfxm::mat4 mat4f(1.0f);
        trans.getOpenGLMatrix((btScalar*)&mat4f);
        
        t->SetTransform(mat4f);
    }
    
    void OnInit()
    {
        dynamics = GetObject()->Root()->GetComponent<Dynamics>();
        
        gfxm::mat4 mat4f(1.0f);
        btTransform trans;
        trans.setFromOpenGLMatrix((btScalar*)&mat4f);
        
        btSphereShape* shape = new btSphereShape(0.5f);
        shape->setMargin(0.0f);
        btDefaultMotionState* motionState =
            new btDefaultMotionState(trans);
        btVector3 inertia(0, 0, 0);
        shape->calculateLocalInertia(1, inertia);
        
        btRigidBody::btRigidBodyConstructionInfo
            rigidBodyCI(1, motionState, shape, inertia);
        
        rigidBody = new btRigidBody(rigidBodyCI);
        
        dynamics->AddRigidBody(this);
        
        UpdateFromTransform();
    }
private:
    btRigidBody* rigidBody;
    Dynamics* dynamics;
};

#endif
