#ifndef COLLIDER_H
#define COLLIDER_H

#include <component.h>
#include <util/gfxm.h>

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

class CollisionShape {
public:
    virtual ~CollisionShape() {}
    virtual btCollisionShape* GetBtShapePtr() = 0;
    virtual gfxm::vec3 GetPivotOffset() { return center; }
    virtual const char* _name() = 0;
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
    virtual const char* _name() { return "Box"; }
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
    virtual const char* _name() { return "Sphere"; }
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


class Collider : public Component {
    RTTR_ENABLE(Component)
public:
    enum TYPE {
        BOX,
        SPHERE,
        CYLINDER,
        CAPSULE,
        CONE,
        PLANE,
        STATIC_PLANE,
        CONCAVE_MESH,
        CONVEX_MESH
    };

    Collider() 
    : shape(std::make_shared<SphereCollisionShape>()) {

    }

    std::shared_ptr<CollisionShape> GetShape() { return shape; }

    virtual bool _write(std::ostream& out, ExportData& exportData) {
        //out.write((char*)&color, sizeof(color));
        //out.write((char*)&intensity, sizeof(intensity));
        return true;
    }
    virtual bool _read(std::istream& in, size_t sz, ImportData& importData) {
        //if(sz != sizeof(color) + sizeof(intensity)) 
        //    return false;
        //in.read((char*)&color, sizeof(color));
        //in.read((char*)&intensity, sizeof(intensity));
        return true;
    }
    virtual bool _editor() {        
        if (ImGui::BeginCombo("Type", type_to_str(type).c_str(), 0)) // The second parameter is the label previewed before opening the combo.
        {
            if(ImGui::Selectable("Box", type == BOX)) { type = BOX; shape.reset(new BoxCollisionShape); Refresh(); }
            if(ImGui::Selectable("Sphere", type == SPHERE)) { type = SPHERE; shape.reset(new SphereCollisionShape); Refresh(); }
            /*
            if(ImGui::Selectable("Cylinder", type == CYLINDER)) { type = CYLINDER; Refresh(); }
            if(ImGui::Selectable("Capsule", type == CAPSULE)) { type = CAPSULE; Refresh(); }
            if(ImGui::Selectable("Cone", type == CONE)) { type = CONE; Refresh(); }
            if(ImGui::Selectable("Plane", type == PLANE)) { type = PLANE; Refresh(); }
            if(ImGui::Selectable("Static plane", type == STATIC_PLANE)) { type = STATIC_PLANE; Refresh(); }
            if(ImGui::Selectable("Concave mesh", type == CONCAVE_MESH)) { type = CONCAVE_MESH; Refresh(); }
            if(ImGui::Selectable("Convex mesh", type == CONVEX_MESH)) { type = CONVEX_MESH; Refresh(); }
            */
            ImGui::EndCombo();
        }

        if(shape) {
            shape->_editor(this);
        }

        return true;
    }
private:
    TYPE type = SPHERE;
    std::shared_ptr<CollisionShape> shape;
    
    std::string type_to_str(TYPE type) {
        switch(type)
        {
        case BOX:
            return "Box";
        case SPHERE:
            return "Sphere";
        case CYLINDER:
            return "Cylinder";
        case CAPSULE:
            return "Capsule";
        case CONE:
            return "Cone";
        case PLANE:
            return "Plane";
        case STATIC_PLANE:
            return "Static plane";
        case CONCAVE_MESH:
            return "Concave mesh";
        case CONVEX_MESH:
            return "Convex mesh";
        default:
            return "UNKNOWN";
        }
    }
};
STATIC_RUN(Collider)
{
    rttr::registration::class_<Collider>("Collider")
        .constructor<>()(rttr::policy::ctor::as_raw_ptr);
}

#endif
