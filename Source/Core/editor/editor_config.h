#ifndef EDITOR_CONFIG_H
#define EDITOR_CONFIG_H

#include <string>
#include <iostream>
#include <external/json.hpp>
#include <fstream>
#include "../../general/util.h"

class EditorConfigObject {
public:
    void Init() {
        using json = nlohmann::json;
        std::ifstream json_file(get_module_dir() + "\\config.json");
        if(!json_file.is_open())
        {
            return;
        }
        json j;
        try
        {
            json_file >> j;
        }
        catch(std::exception& ex)
        {
            std::cout << ex.what();
            return;
        }
        if(!j.is_object())
        {
            std::cout << "config json is not object";
            return;
        }

        if(j["project_config"].is_string())
        {
            projectConfPath = j["project_config"].get<std::string>();
            projectRoot = cut_dirpath(projectConfPath);
        }
        if(j["builder"].is_string())
        {
            builderPath = j["builder"].get<std::string>();
        }
    }

    std::string projectConfPath;
    std::string builderPath;
    std::string projectRoot;
};

inline EditorConfigObject& EditorConfig() {
    static EditorConfigObject cfg = EditorConfigObject();
    return cfg;
}


#endif
