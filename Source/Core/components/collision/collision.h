#ifndef COM_COLLISION_H
#define COM_COLLISION_H

#include <util/gfxm.h>
#include <aurora/gfx.h>
#include "../../scene_object.h"

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <LinearMath/btIDebugDraw.h>

#include <set>

class CollisionDebugDraw : public btIDebugDraw
{
public:
    virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
    {
        glBegin(GL_LINE);
        glColor3f(color.x(), color.y(), color.z());
        glVertex3f(from.x(), from.y(), from.z());
        glVertex3f(to.x(), to.y(), to.z());
        glEnd();
    }
    
    virtual void drawContactPoint(const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int lifeTime, const btVector3 &color)
    {
        
    }
    
    virtual void reportErrorWarning(const char* warningString)
    {
        
    }
    
    virtual void draw3dText(const btVector3& location, const char* textString)
    {
        
    }
    
    virtual void setDebugMode(int mode)
    {
        
    }
    
    virtual int getDebugMode() const { return 0; }
};

class Collider;
class GhostCollider;
class ConvexCollider;
class ConvexGhostCollider;
class KinematicObject;
class Collision : public SceneObject::Component
{
    CLONEABLE(Collision)
    RTTR_ENABLE(SceneObject::Component)
public:
    struct RayHit
    {
        SceneObject* object;
        gfxm::vec3 position;
        gfxm::vec3 normal;
    };
    
	SceneObject* RayTest(gfxm::ray& ray)
	{
		RayHit hit;
		return RayTest(ray, hit);
	}
    
	SceneObject* RayTest(gfxm::ray& ray, RayHit& hit)
	{
        hit.object = 0;
        
		btVector3 origin = btVector3(ray.origin.x, ray.origin.y, ray.origin.z);
		btVector3 direction = btVector3(ray.direction.x, ray.direction.y, ray.direction.z);
		btVector3 hitPoint;
		btVector3 btHitNormal;
		
		btCollisionWorld::ClosestRayResultCallback rayCb(origin, direction);
		world->rayTest(
			origin,
			origin + direction,
			rayCb
		);
		
		if(rayCb.hasHit())
		{
			hitPoint = rayCb.m_hitPointWorld;
			hitPoint.setInterpolate3(origin, origin + direction, rayCb.m_closestHitFraction);
			hit.position = gfxm::vec3(hitPoint.x(), hitPoint.y(), hitPoint.z());
			btHitNormal = rayCb.m_hitNormalWorld;
            hit.normal = gfxm::vec3(btHitNormal.x(), btHitNormal.y(), btHitNormal.z());
			const btCollisionObject* co = rayCb.m_collisionObject;
			SceneObject* c = (SceneObject*)co->getUserPointer();
            hit.object = c;
			return c;
		}
		
		return 0;
	}
    
    //gfxm::vec3 SweepTest(ConvexGhostCollider* collider, gfxm::mat4& from, gfxm::mat4& to);
	
	void AddCollider(Collider* col);
    void RemoveCollider(Collider* col);
    void AddKinematicObject(KinematicObject* ko);
    void RemoveKinematicObject(KinematicObject* ko);
    
    void Update(float dt);
    
    // Should be removed
    btCollisionWorld* GetBtWorld() { return world; }

	virtual void OnInit()
	{
		collisionConf = new btDefaultCollisionConfiguration();
        dispatcher = new btCollisionDispatcher(collisionConf);
        broadphase = new btDbvtBroadphase();
		
		world = new btCollisionWorld(
            dispatcher, 
            broadphase,
            collisionConf
        );
        
        world->setDebugDrawer(&debugDrawer);
	}
    
    void DebugDraw()
    {
        world->debugDrawWorld();
    }
private:
    CollisionDebugDraw debugDrawer;
    std::set<Collider*> colliders;
    std::set<KinematicObject*> kinematics;

	btDefaultCollisionConfiguration* collisionConf;
    btCollisionDispatcher* dispatcher;
    btDbvtBroadphase* broadphase;
	btCollisionWorld* world;
};

#endif
