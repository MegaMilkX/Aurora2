#ifndef COMPONENT_CAMERA_H
#define COMPONENT_CAMERA_H

#include <aurora/gfx.h>

#include "transform.h"
#include "../scene_object.h"
#include "renderer.h"

#undef GetObject

class Camera : public SceneObject::Component
{
    CLONEABLE(Camera)
    RTTR_ENABLE(SceneObject::Component)
public:
    Camera()
    : fov(1.6f), aspect(16.0f/9.0f), zNear(0.1f), zFar(100.0f)
    {
        Perspective(fov, aspect, zNear, zFar);
    }
    ~Camera()
    {

    }
    
    void Ortho(){}
    void Perspective(float fov, float aspect, float zNear, float zFar)
    {
        this->fov = fov;
        this->aspect = aspect;
        this->zNear = zNear;
        this->zFar = zFar;
        projection = gfxm::perspective(fov, aspect, zNear, zFar);
    }

    gfxm::vec2 WorldToScreen(const gfxm::vec3& w)
    {
        gfxm::vec4 world4(w.x, w.y, w.z, 1.0f);
        world4 = Projection() * InverseTransform() * world4;
        return gfxm::vec2(world4.x / world4.w, world4.y / world4.w);
    }
    
    gfxm::mat4 Projection() { return projection; }
    gfxm::mat4 InverseTransform() { return gfxm::inverse(transform->GetTransform()); }

    virtual void OnInit()
    {
        transform = GetObject()->GetComponent<Transform>();
    }
private:
    float fov;
    float aspect;
    float zNear;
    float zFar;
    gfxm::mat4 projection;

    Transform* transform;
    Renderer* renderer;
};
STATIC_RUN(Camera)
{
    rttr::registration::class_<Camera>("Camera")
        .constructor<>()(rttr::policy::ctor::as_raw_ptr);
}

#endif
