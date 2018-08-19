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
#include <resource_object.h>

class Material : public ResourceObject
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
    
    bool Build(ResourceRaw* r)
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
    
private:
    std::map<std::string, std::string> strstr;
};

#endif
