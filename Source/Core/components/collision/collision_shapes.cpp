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

void TriangleMeshCollisionShape::SetMesh(std::shared_ptr<Mesh> mesh) {
    vertices = mesh->GetAttribBytes<Au::Position>();
    indices = mesh->GetIndices();

    gfxm::vec3* vertices_ptr = (gfxm::vec3*)vertices.data();
    for(uint32_t i = 0; i < mesh->GetAttribCount<Au::Position>(); ++i) {
        gfxm::vec4 v(
            vertices_ptr[i].x,
            vertices_ptr[i].y,
            vertices_ptr[i].z,
            1.0f
        );
        vertices_ptr[i] = physical_object->Get<Transform>()->GetTransform() * v;
    }

    btTriangleIndexVertexArray* indexVertexArray = new btTriangleIndexVertexArray(
        mesh->GetIndexCount() / 3,
        (int32_t*)indices.data(),
        sizeof(uint32_t) * 3,
        mesh->GetAttribCount<Au::Position>(),
        (btScalar*)vertices.data(),
        sizeof(float) * 3
    );
    shape.reset(new btBvhTriangleMeshShape(
        indexVertexArray, true
    ));
    physical_object->OnShapeChange();
}

void TriangleMeshCollisionShape::_editor(Component* c) {
    ButtonResource("Mesh", mesh, ".geo", [this](std::shared_ptr<Mesh> mesh){
        SetMesh(mesh);
    });
    if(ImGui::Button("Set from model")) {
        Model* m = physical_object->Object()->FindComponent<Model>();
        if(m) {
            if(m->mesh) {
                SetMesh(m->mesh);
            }
        }
    }
}