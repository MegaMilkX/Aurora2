#ifndef COMPONENT_MATERIAL_H
#define COMPONENT_MATERIAL_H

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <set>

#include <aurora/glslstitch.h>
#include <aurora/lua.h>

#include "../component.h"

#include <resource.h>

#include "texture2d.h"

#include "../external/json.hpp"

#include <resources/resource/resource_factory.h>

class Material : public Resource
{
    RTTR_ENABLE(Resource)
public:    
    Material() {}

    std::shared_ptr<Texture2D> diffuseMap;
    std::shared_ptr<Texture2D> normalMap;
    std::shared_ptr<Texture2D> specularMap;
    gfxm::vec3 tint = gfxm::vec3(1.0f, 1.0f, 1.0f);
    float glossiness = 0.0f;
    float emission = 0.0f;
    
    void Set(const std::string& key, const std::string& value)
    {
        strstr[key] = value;
        if(key == "Diffuse" || key == "DiffuseMap") {
            diffuseMap = GlobalResourceFactory().Get<Texture2D>(value);
        } else if(key == "Normal" || key == "NormalMap") {
            normalMap = GlobalResourceFactory().Get<Texture2D>(value);
        } else if(key == "Specular" || key == "SpecularMap") {
            specularMap = GlobalResourceFactory().Get<Texture2D>(value);
        }
    }

    std::string GetString(const std::string& key) { return strstr[key]; }
    
    bool Build(DataSourceRef r)
    {
        std::vector<char> data;
        if(r->Size() == 0) return false;
        data.resize((size_t)r->Size());
        r->ReadAll((char*)data.data());

        using json = nlohmann::json;
        json j;
        try
        {
            j = json::parse((char*)data.data(), (char*)data.data() + data.size());
        }
        catch(std::exception& e)
        {
            std::cout << "Material json parse error: " << e.what() << std::endl;
            return false;
        }

        for(json::iterator it = j.begin(); it != j.end(); ++it)
        {
            if(it.value().is_string())
            {
                Set(it.key(), it.value().get<std::string>());
            }
        }

        return true;
    }
    virtual bool Serialize(std::vector<unsigned char>& data) {
        LOG("Material::Serialize - NOT IMPLEMENTED");
        return false;
    }
private:
    std::map<std::string, std::string> strstr;
};
STATIC_RUN(Material)
{
    rttr::registration::class_<Material>("Material")
        .constructor<>()(rttr::policy::ctor::as_raw_ptr);
}

#endif
