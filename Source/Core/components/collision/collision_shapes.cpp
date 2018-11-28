#include "../physical_object.h"

void BoxCollisionShape::_editor(Component* c) {
    if(ImGui::DragFloat3("Size", (float*)&size, 0.001f)) {
        shape = btBoxShape(btVector3(size.x, size.y, size.z));
        physical_object->OnShapeChange();
    }
    if(ImGui::DragFloat3("Pivot offset", (float*)&center, 0.001f)) {
        physical_object->OnShapeChange();
    }
}

void SphereCollisionShape::_editor(Component* c) {
    if(ImGui::DragFloat("Radius", &radius, 0.001f)) {
        shape = btSphereShape(radius);
        physical_object->OnShapeChange();
    }
    if(ImGui::DragFloat3("Pivot offset", (float*)&center, 0.001f)) {
        physical_object->OnShapeChange();
    }
}