#ifndef SCENE_PHYSICS_H
#define SCENE_PHYSICS_H

#include <util/gfxm.h>
#include <LinearMath/btIDebugDraw.h>
#include "external/imguizmo/imguizmo.h"
#include <collider.h>
#include <rigid_body.h>
#include "debug_draw.h"

class BulletDebugDrawer_OpenGL : public btIDebugDraw {
public:
	virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) 
	{
        const ddVec3 f  = { from.getX(), from.getY(), from.getZ() };
        const ddVec3 t = { to.getX(), to.getY(), to.getZ() };
        const ddVec3 col = { color.getX(), color.getY(), color.getZ() };
        dd::line(f, t, col, 0, false);
	}
	virtual void drawContactPoint(const btVector3 &, const btVector3 &, btScalar, int, const btVector3 &) {}
	virtual void reportErrorWarning(const char *) {}
	virtual void draw3dText(const btVector3 & pos, const char * text) {
        const ddVec3 p = {pos.getX(), pos.getY(), pos.getZ()};
        const ddVec3 c = { 1.0f, 1.0f, 1.0f };
        dd::screenText(text, p, c);
    }
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
		/*
        world = new btCollisionWorld(
            dispatcher, 
            broadphase,
            collisionConf
        );
        */
        constraintSolver = new btSequentialImpulseConstraintSolver();
        world = new btDiscreteDynamicsWorld(
            dispatcher, 
            broadphase, 
            constraintSolver,
            collisionConf
        );
        world->setGravity(btVector3(0.0f, -10.0f, 0.0f));

        world->setDebugDrawer(&debugDrawer);
    }
    void Update() {
        for(auto kv : objects) {
            Transform* t = kv.first->GetComponent<Transform>();
            //gfxm::mat4 m = gfxm::translate(gfxm::mat4(1.0f), t->WorldPosition()) * 
            //    gfxm::to_mat4(t->WorldRotation()) *
            //    gfxm::translate(gfxm::mat4(1.0f), ((Collider*)kv.first)->GetShape()->GetPivotOffset());
            gfxm::mat4 m = t->GetTransform() * 
                gfxm::translate(gfxm::mat4(1.0f), ((Collider*)kv.first)->GetShape()->GetPivotOffset());
            btTransform trans;
            trans.setFromOpenGLMatrix((btScalar*)&m);
            kv.second->setWorldTransform(trans);
        }
    }
    void DebugDraw() {
        world->debugDrawWorld();
    }

    void _onAddComponent(rttr::type type, Component* c, SceneObject* so) {
        if(type == rttr::type::get<Collider>()) {
            objects[c] = new btCollisionObject();
            objects[c]->setCollisionShape(((Collider*)c)->GetShape()->GetBtShapePtr());
            world->addCollisionObject(objects[c]);
        } else if(type == rttr::type::get<RigidBody>()) {

        }
    }
    void _onRemoveComponent(rttr::type type, Component* c, SceneObject* so) {
        if(type == rttr::type::get<Collider>()) {
            if(objects.count(c) != 0) {
                world->removeCollisionObject(objects[c]);
                delete objects[c];
                objects.erase(c);
            }
        } else if(type == rttr::type::get<RigidBody>()) {
            
        }
    }
private:
    std::map<Component*, btCollisionObject*> objects;

    btDefaultCollisionConfiguration* collisionConf;
    btCollisionDispatcher* dispatcher;
    btDbvtBroadphase* broadphase;
	//btCollisionWorld* world;
    btSequentialImpulseConstraintSolver* constraintSolver;
    btDiscreteDynamicsWorld* world;

    BulletDebugDrawer_OpenGL debugDrawer;
};

#endif
