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
    void Set(const std::string& key, const gfxm::vec3& value) {
        if(key == "Tint") {
            tint = value;
        }
    }
    void Set(const std::string& key, float value) {
        if(key == "Glossiness") {
            glossiness = value;
        } else if(key == "Emission") {
            emission = value;
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
            if(it.value().is_string()) {
                Set(it.key(), it.value().get<std::string>());
            } else if(it.value().is_number()) {
                Set(it.key(), it.value().get<float>());
            } else if(it.value().is_array()) {
                if(it.value().size() == 3) {
                    if(it.value()[0].is_number()
                    && it.value()[1].is_number()
                    && it.value()[2].is_number()) {
                        Set(it.key(), gfxm::vec3(
                            it.value()[0].get<float>(),
                            it.value()[1].get<float>(),
                            it.value()[2].get<float>()
                        ));
                    }
                }
            }
        }

        return true;
    }
    virtual bool Serialize(std::vector<unsigned char>& data) {
        nlohmann::json j;
        if(diffuseMap) {
            j["DiffuseMap"] = diffuseMap->Name();
        }
        if(normalMap) {
            j["NormalMap"] = normalMap->Name();
        }
        if(specularMap) {
            j["SpecularMap"] = specularMap->Name();
        }
        j["Tint"] = { tint.x, tint.y, tint.z };
        j["Glossiness"] = glossiness;
        j["Emission"] = emission;

        std::string jstring = j.dump();
        data = std::vector<unsigned char>((unsigned char*)jstring.data(), (unsigned char*)jstring.data() + jstring.size());
        
        return true;
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
