#ifndef COMPONENT_CAMERA_H
#define COMPONENT_CAMERA_H

#include <aurora/gfx.h>

#include "transform.h"
#include "../scene_object.h"
#include <component.h>
#include "renderer.h"

#undef GetObject

class Camera : public SceneObject::Component
{
public:
    Camera()
    : fov(1.6f), aspect(16.0f/9.0f), zNear(0.1f), zFar(100.0f)
    {
        Perspective(fov, aspect, zNear, zFar);
    }
    ~Camera()
    {
        renderer->CurrentCamera(renderer->GetObject()->GetComponent<Camera>());
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

    void Render()
    {
        renderer->Render(
            projection,
            transform->GetTransform()
        );
    }

    virtual void OnInit()
    {
        transform = GetObject()->GetComponent<Transform>();
        renderer = GetObject()->Root()->GetComponent<Renderer>();
        
        renderer->CurrentCamera(this);
    }
    virtual std::string Serialize() 
    {
        using json = nlohmann::json;
        json j = json::object();
        j["fov"] = fov;
        j["aspect"] = aspect;
        j["znear"] = zNear;
        j["zfar"] = zFar;
        return j.dump(); 
    }
    virtual void Deserialize(const std::string& data)
    {
        using json = nlohmann::json;
        json j = json::parse(data);
        if(j.is_null())
            return;
        if(j["fov"].is_number())
            fov = j["fov"].get<float>();
        if(j["aspect"].is_number())
            aspect = j["aspect"].get<float>();
        if(j["znear"].is_number())
            zNear = j["znear"].get<float>();
        if(j["zfar"].is_number())
            zFar = j["zfar"].get<float>();
        Perspective(fov, aspect, zNear, zFar);
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
COMPONENT(Camera)

#endif
