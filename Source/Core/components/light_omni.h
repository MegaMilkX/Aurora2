#ifndef COMPONENT_LIGHT_OMNI_H
#define COMPONENT_LIGHT_OMNI_H

#include "../scene_object.h"
#include <component.h>
#include "renderer.h"

#include <util/gfxm.h>

class LightDirect : public SceneObject::Component
{
public:
    ~LightDirect()
    {
    }
    
    void Color(float r, float g, float b)
    { color = gfxm::vec3(r, g, b); }
    
    gfxm::vec3 Color()
    { return color; }
    
    void Direction(float x, float y, float z)
    { direction = gfxm::vec3(x, y, z); }
    gfxm::vec3 Direction()
    { return direction; }
    
    void OnInit()
    {
    }
    virtual std::string Serialize() 
    { 
        using json = nlohmann::json;
        json j = json::object();
        j["Color"] = {color.x, color.y, color.z};
        j["Direction"] = {direction.x, direction.y, direction.z};
        return j.dump(); 
    }
    virtual void Deserialize(const std::string& data)
    {
        using json = nlohmann::json;
        json j = json::parse(data);
        if(j.is_null())
            return;
        if(j["Color"].is_array() && j["Color"].size() == 3)
        {
            Color(
                j["Color"][0].get<float>(),
                j["Color"][1].get<float>(),
                j["Color"][2].get<float>()
            );
        }
        if(j["Direction"].is_array() && j["Direction"].size() == 3)
        {
            Direction(
                j["Direction"][0].get<float>(),
                j["Direction"][1].get<float>(),
                j["Direction"][2].get<float>()
            );
        }
    }
private:
    gfxm::vec3 color;
    gfxm::vec3 direction;
};
COMPONENT(LightDirect)

class LightOmni : public SceneObject::Component
{
public:
    ~LightOmni()
    {
    }

    void Color(float r, float g, float b)
    {
        color = gfxm::vec3(r, g, b);
    }
    void Intensity(float i)
    {
        intensity = i;
    }
    
    gfxm::vec3 Color()
    { return color; }

    void OnInit()
    {
    }
    
    virtual std::string Serialize() 
    { 
        using json = nlohmann::json;
        json j = json::object();
        j["Color"] = {color.x, color.y, color.z};
        j["Intensity"] = intensity;
        return j.dump(); 
    }
    virtual void Deserialize(const std::string& data)
    {
        using json = nlohmann::json;
        json j = json::parse(data);
        if(j.is_null())
            return;
        if(j["Color"].is_array() && j["Color"].size() == 3)
        {
            Color(
                j["Color"][0].get<float>(),
                j["Color"][1].get<float>(),
                j["Color"][2].get<float>()
            );
        }
        if(j["Intensity"].is_number())
        {
            Intensity(j["Intensity"].get<float>());
        }
    }
private:
    gfxm::vec3 color;
    float intensity;
};
COMPONENT(LightOmni)

#endif
