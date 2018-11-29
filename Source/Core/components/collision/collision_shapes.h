#ifndef COLLISION_SHAPES_H
#define COLLISION_SHAPES_H

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include <mesh.h>
#include <model.h>
#include <transform.h>
#include <util/editor_gui_helpers.h>

class PhysicalObject;

class CollisionShape {
    friend PhysicalObject;
public:
    virtual ~CollisionShape() {}
    virtual btCollisionShape* GetBtShapePtr() = 0;
    virtual gfxm::vec3 GetPivotOffset() { return center; }
    virtual void _editor(Component* c) {}
protected:
    PhysicalObject* physical_object;
    gfxm::vec3 center = gfxm::vec3(0.0f, 0.0f, 0.0f);
};

class BoxCollisionShape : public CollisionShape {
public:
    BoxCollisionShape()
    : shape(btVector3(0.5f, 0.5f, 0.5f)) {
        
    }
    btBoxShape shape;
    gfxm::vec3 size = gfxm::vec3(0.5f, 0.5f, 0.5f);

    virtual btCollisionShape* GetBtShapePtr() {
        return &shape;
    }
    virtual void _editor(Component* c);
};

class SphereCollisionShape : public CollisionShape {
public:
    SphereCollisionShape()
    : shape(0.5f) {

    }
    btSphereShape shape;
    float radius = 0.5f;

    virtual btCollisionShape* GetBtShapePtr() {
        return &shape;
    }
    virtual void _editor(Component* c);
};

class TriangleMeshCollisionShape : public CollisionShape {
public:
    TriangleMeshCollisionShape() {
        shape.reset(new btEmptyShape());
    }
    std::shared_ptr<btCollisionShape> shape;
    std::shared_ptr<Mesh> mesh;
    std::vector<unsigned char> vertices;
    std::vector<uint32_t> indices;

    void SetMesh(std::shared_ptr<Mesh> mesh);

    virtual btCollisionShape* GetBtShapePtr() {
        return shape.get();
    }
    virtual void _editor(Component* c);
};

#endif
