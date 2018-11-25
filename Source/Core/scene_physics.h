#ifndef SCENE_PHYSICS_H
#define SCENE_PHYSICS_H

#include <util/gfxm.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <LinearMath/btIDebugDraw.h>
#include <component.h>

class ScenePhysics {
public:
    void Init() {
        collisionConf = new btDefaultCollisionConfiguration();
        dispatcher = new btCollisionDispatcher(collisionConf);
        broadphase = new btDbvtBroadphase();
		world = new btCollisionWorld(
            dispatcher, 
            broadphase,
            collisionConf
        );
    }
    void Update() {

    }
    void _onAddComponent(rttr::type type, Component* c, SceneObject* so) {
    
    }
    void _onRemoveComponent(rttr::type type, Component* c, SceneObject* so) {
    
    }
private:
    btDefaultCollisionConfiguration* collisionConf;
    btCollisionDispatcher* dispatcher;
    btDbvtBroadphase* broadphase;
	btCollisionWorld* world;
};

#endif
