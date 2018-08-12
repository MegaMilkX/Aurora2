#ifndef COM_PHYSICS_H
#define COM_PHYSICS_H

#include <vector>
#include <util/gfxm.h>
#include "../../scene_object.h"

#include <btBulletDynamicsCommon.h>

class RigidBody;
class Dynamics : public SceneObject::Component
{
    CLONEABLE(Dynamics)
    RTTR_ENABLE(SceneObject::Component)
public:
    Dynamics(){}
    ~Dynamics(){}
	
    void Step(float dt);
    
    void AddRigidBody(RigidBody* rb);
    void RemoveRigidBody(RigidBody* rb);
    
    virtual void OnInit()
    {
        collisionConf = new btDefaultCollisionConfiguration();
        dispatcher = new btCollisionDispatcher(collisionConf);
        broadphase = new btDbvtBroadphase();
        constraintSolver = new btSequentialImpulseConstraintSolver();
        
        world = new btDiscreteDynamicsWorld(
            dispatcher, 
            broadphase, 
            constraintSolver,
            collisionConf
        );
        
        world->setGravity(btVector3(0.0f, -10.0f, 0.0f));
        
        // Ground plane
        btCollisionShape* plane = new btStaticPlaneShape(btVector3(0, 1, 0), 0);
        btDefaultMotionState* planeMotionState = 
            new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(0, 0, 0)));
        
        btRigidBody::btRigidBodyConstructionInfo
            rigidBodyCI(0, planeMotionState, plane, btVector3(0,0,0));
        btRigidBody* rigidBody = new btRigidBody(rigidBodyCI);
        world->addRigidBody(rigidBody);
    }
private:
    std::vector<RigidBody*> rigidBodies;

    btDefaultCollisionConfiguration* collisionConf;
    btCollisionDispatcher* dispatcher;
    btDbvtBroadphase* broadphase;
    btSequentialImpulseConstraintSolver* constraintSolver;
    btDiscreteDynamicsWorld* world;
};

#endif
