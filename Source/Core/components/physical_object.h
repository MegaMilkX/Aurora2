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
    {}

    template<typename T>
    std::shared_ptr<CollisionShape> SetShape() { 
        shape.reset(new T());
        shape_type = rttr::type::get<T>();
        Refresh();
        return shape; 
    }
    std::shared_ptr<CollisionShape> GetShape() { return shape; }
protected:
    std::shared_ptr<CollisionShape> shape;
    rttr::type shape_type;
};

#endif
