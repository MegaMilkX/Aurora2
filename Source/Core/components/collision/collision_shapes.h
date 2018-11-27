#ifndef COLLISION_SHAPES_H
#define COLLISION_SHAPES_H

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

class CollisionShape {
public:
    virtual ~CollisionShape() {}
    virtual btCollisionShape* GetBtShapePtr() = 0;
    virtual gfxm::vec3 GetPivotOffset() { return center; }
    virtual void _editor(Component* c) {}
protected:
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
    virtual void _editor(Component* c) {
        if(ImGui::DragFloat3("Size", (float*)&size, 0.001f)) {
            shape = btBoxShape(btVector3(size.x, size.y, size.z));
            c->Refresh();
        }
        if(ImGui::DragFloat3("Pivot offset", (float*)&center, 0.001f)) {
            c->Refresh();
        }
    }
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
    virtual void _editor(Component* c) {
        if(ImGui::DragFloat("Radius", &radius, 0.001f)) {
            shape = btSphereShape(radius);
            c->Refresh();
        }
        if(ImGui::DragFloat3("Pivot offset", (float*)&center, 0.001f)) {
            c->Refresh();
        }
    }
};

#endif
