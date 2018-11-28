#ifndef COLLISION_OBJECT_H
#define COLLISION_OBJECT_H

#include <component.h>
#include "collision/collision_shapes.h"

class PhysicalObject : public Component {
    RTTR_ENABLE(Component)
public:
    PhysicalObject()
    : shape(new BoxCollisionShape()),
    shape_type(rttr::type::get<BoxCollisionShape>()) 
    {
        shape->physical_object = this;
    }

    template<typename T>
    std::shared_ptr<CollisionShape> SetShape() { 
        shape.reset(new T());
        shape->physical_object = this;
        shape_type = rttr::type::get<T>();
        OnShapeChange();
        return shape; 
    }
    std::shared_ptr<CollisionShape> GetShape() { return shape; }

    virtual void OnShapeChange() = 0;
protected:
    std::shared_ptr<CollisionShape> shape;
    rttr::type shape_type;
};

#endif
