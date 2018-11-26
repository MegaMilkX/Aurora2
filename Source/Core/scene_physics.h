#ifndef SCENE_PHYSICS_H
#define SCENE_PHYSICS_H

#include <util/gfxm.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <LinearMath/btIDebugDraw.h>
#include "external/imguizmo/imguizmo.h"
#include <component.h>
#include "debug_draw.h"

class BulletDebugDrawer_OpenGL : public btIDebugDraw {
public:
	virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) 
	{
        const ddVec3 f  = { from.getX(), from.getY(), from.getZ() };
        const ddVec3 t = { to.getX(), to.getY(), to.getZ() };
        const ddVec3 col = { 1.0f, 1.0f, 1.0f };
        dd::line(f, t, col, 0, false);
	}
	virtual void drawContactPoint(const btVector3 &, const btVector3 &, btScalar, int, const btVector3 &) {}
	virtual void reportErrorWarning(const char *) {}
	virtual void draw3dText(const btVector3 &, const char *) {}
	virtual void setDebugMode(int p) {
		m = p;
	}
	int getDebugMode(void) const { return 3; }
	int m;
};

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

        world->setDebugDrawer(&debugDrawer);

        //
        collisionObject = new btCollisionObject();
        shape = new btSphereShape(0.5f);
		collisionObject->setCollisionShape(shape);
        world->addCollisionObject(collisionObject);
    }
    void Update() {

    }
    void DebugDraw() {
        world->debugDrawWorld();
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

    BulletDebugDrawer_OpenGL debugDrawer;

    //
    btCollisionObject* collisionObject;
	btCollisionShape* shape;
};

#endif
