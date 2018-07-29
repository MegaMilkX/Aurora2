#ifndef COMPONENT_MATERIAL_H
#define COMPONENT_MATERIAL_H

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <set>

#include <aurora/glslstitch.h>
#include <aurora/lua.h>

#include "../scene_object.h"
#include "renderer.h"

#include <resource.h>

#include "texture2d.h"

#include "../external/json.hpp"

#include <asset.h>

class Material
{
public:    
    Material()
    {
        
    }
    
    void Set(const std::string& key, const std::string& value)
    {
        strstr[key] = value;
    }

    std::string GetString(const std::string& key) { return strstr[key]; }

    bool Build(Resource* r)
    {
        using json = nlohmann::json;
        json j;
        try
        {
            j = json::parse((char*)r->Data());
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
private:
    std::map<std::string, std::string> strstr;
};

template<>
inline bool LoadAsset<Material, JSON>(Material* material, const std::string& filename)
{
    using json = nlohmann::json;

    std::ifstream file(filename, std::ios::in);
    if(!file.is_open())
        return false;

    json j;
    try
    {
        j = json::parse(file);
    }
    catch(std::exception& e)
    {
        std::cout << "Material json parse error: " << e.what() << std::endl;
        file.close();
        return false;
    }

    for(json::iterator it = j.begin(); it != j.end(); ++it)
    {
        if(it.value().is_string())
        {
            material->Set(it.key(), it.value().get<std::string>());
        }
    }

    
    file.close();
    
    return true;
}

#endif
