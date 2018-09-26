#ifndef COM_COLLIDER_H
#define COM_COLLIDER_H

#include "../transform.h"
#include "collision.h"
#include <mesh.h>

#undef GetObject

class Collider : public Component
{
    CLONEABLE(Collider)
    RTTR_ENABLE(Component)
friend Collision;
public:
    Collider()
    {
        offset = gfxm::vec3(0.0f, 0.0f, 0.0f);
    }
    
	virtual ~Collider()
	{
		collision->RemoveCollider(this);
	}
    
    void SetOffset(float x, float y, float z) 
    { 
        offset = gfxm::vec3(x, y, z);
        UpdateTransform();
    }
    gfxm::vec3 GetOffset() { return offset; }
    
    void UpdateTransform()
    {
        gfxm::mat4 mat4f = transform->GetTransform();
        mat4f = gfxm::translate(mat4f, offset);
		btTransform trans;
        trans.setFromOpenGLMatrix((btScalar*)&mat4f);
		collisionObject->setWorldTransform(trans);
    }
    
    btCollisionObject* GetBtObject() { return collisionObject; }
    
	virtual void OnInit()
	{
        transform = Get<Transform>();
		collision = GetObject()->Root()->GetComponent<Collision>();

		collisionObject = new btCollisionObject();		
		collisionObject->setUserPointer(GetObject());
		
		UpdateTransform();
	}
protected:
	btCollisionObject* collisionObject;
	btCollisionShape* shape;
	
    Transform* transform;
	Collision* collision;
    
    gfxm::vec3 offset;
};

class ConvexCollider : public Collider
{
    CLONEABLE(ConvexCollider)
    RTTR_ENABLE(Collider)
friend Collision;
public:
    virtual void OnInit()
    {
        Collider::OnInit();
    }
protected:
    btConvexShape* convexShape;
};

class PlaneCollider : public Collider
{    
    CLONEABLE(PlaneCollider)
    RTTR_ENABLE(Collider)
public:
	virtual void OnInit()
	{
		Collider::OnInit();
		shape = new btStaticPlaneShape(btVector3(0.0f, 1.0f, 0.0f), 0.0f);
		collisionObject->setCollisionShape(shape);
		collision->AddCollider(this);
	}
};

class SphereCollider : public Collider
{
    CLONEABLE(SphereCollider)
    RTTR_ENABLE(Collider)
public:
	virtual void OnInit()
	{
		Collider::OnInit();
		shape = new btSphereShape(0.5f);
		collisionObject->setCollisionShape(shape);
		collision->AddCollider(this);
	}
};

class CapsuleCollider : public ConvexCollider
{
    CLONEABLE(CapsuleCollider)
    RTTR_ENABLE(ConvexCollider)
public:
    virtual void OnInit()
    {
        ConvexCollider::OnInit();
        convexShape = new btCapsuleShape(1.0f, 2.0f);
        shape = convexShape;
        collisionObject->setCollisionShape(shape);
        collision->AddCollider(this);
    }
};

class MeshCollider : public Collider
{
    CLONEABLE(MeshCollider)
    RTTR_ENABLE(Collider)
public:
    virtual void OnInit()
    {
        Collider::OnInit();
        
    }
    /*
    void SetMesh(const std::string& resourceName)
    { SetMesh(asset<Mesh>::get(resourceName)); }
    
    void SetMesh(asset<Mesh> meshData)
    {
        if(!meshData)
            return;
        this->meshData = meshData;
        
        int attrCount = meshData->GetAttribCount<Au::Position>();
        Au::Position* posData = meshData->GetAttribData<Au::Position>();
        std::vector<unsigned>& indices = meshData->GetIndices();
        if(!posData || !attrCount)
            return;
        
        triMesh = new btTriangleMesh();
        for(unsigned i = 0; i < indices.size(); i += 3)
        {
            triMesh->addTriangle(
                btVector3(posData[indices[i]][0], posData[indices[i]][1], posData[indices[i]][2]),
                btVector3(posData[indices[i + 1]][0], posData[indices[i + 1]][1], posData[indices[i + 1]][2]),
                btVector3(posData[indices[i + 2]][0], posData[indices[i + 2]][1], posData[indices[i + 2]][2])
            );
        }
        
        shape = new btBvhTriangleMeshShape(triMesh, true);
        collisionObject->setCollisionShape(shape);
        
        collision->AddCollider(this);
    }
    */
private:
    btTriangleMesh* triMesh;
    //asset<Mesh> meshData;
};

#endif
