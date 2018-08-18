#ifndef KINEMATIC_OBJECT_H
#define KINEMATIC_OBJECT_H

#include "../transform.h"
#include "collision.h"
#include "collider.h"

#undef GetObject

typedef gfxm::vec3 vec3;
typedef gfxm::mat4 mat4;

class KinematicObject : public SceneObject::Component
{
    CLONEABLE(KinematicObject)
    RTTR_ENABLE(SceneObject::Component)
public:
    ~KinematicObject()
    {
        GetObject()->Root()->GetComponent<Collision>()->RemoveKinematicObject(this);
    }
    void ResolveCollision()
    {
        vec3 backNormal(0.0f, 0.0f, 0.0f);
        int numManifolds = collision->GetBtWorld()->getDispatcher()->getNumManifolds();
        for (int i = 0; i < numManifolds; i++)
        {
            btPersistentManifold* contactManifold =  collision->GetBtWorld()->getDispatcher()->getManifoldByIndexInternal(i);
            const btCollisionObject* obA = contactManifold->getBody0();
            const btCollisionObject* obB = contactManifold->getBody1();
            
            if(GetObject() == obA->getUserPointer() ||
                GetObject() == obB->getUserPointer())
            {
                int numContacts = contactManifold->getNumContacts();
                for (int j = 0; j < numContacts; j++)
                {
                    btManifoldPoint& pt = contactManifold->getContactPoint(j);
                    if (pt.getDistance() < 0.f)
                    {
                        const btVector3& ptA = pt.getPositionWorldOnA();
                        const btVector3& ptB = pt.getPositionWorldOnB();
                        const btVector3& normalOnB = pt.m_normalWorldOnB;
                        vec3 n(normalOnB.x(), normalOnB.y(), normalOnB.z());
                        n = n * -pt.getDistance();
                        
                        if(GetObject() == obA->getUserPointer())
                        {
                            backNormal = backNormal + n;
                        }
                        else
                        {
                            backNormal = backNormal - n;
                        }
                    }
                }
            }
        }
        
        //backNormal = gfxm::normalize(backNormal);
        //backNormal = backNormal * actor->Velocity().length() * dt;
        //backNormal.y = 0.0f;
        trans->Translate(backNormal);
    }
    
    virtual void Update(float dt)
    {
        
    }

    virtual void OnInit()
    {
        trans = GetComponent<Transform>();
        collision = GetObject()->Root()->GetComponent<Collision>();
        collision->AddKinematicObject(this);
    }
protected:
    Transform* trans;
    Collision* collision;
};

#endif
