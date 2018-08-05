#include "resource_compiler_impl.h"
ResourceCompilerImpl resource_compiler;

using json = nlohmann::json;

bool load_resources(const std::string& path, json& j)
{
    std::ifstream json_file(path);
    try
    {
        json_file >> j;
    }
    catch(std::exception& ex)
    {
        LOG(ex.what());
        return false;
    }
    if(!j.is_object())
    {
        LOG("Resource json is not an object");
        return false;
    }

    return true;
}

bool load_project_json(const std::string& path, project_config& conf)
{
    std::ifstream json_file(path);
    json j;
    try
    {
        json_file >> j;
    }
    catch(std::exception& ex)
    {
        std::cout << ex.what();
        return false;
    }
    if(!j.is_object())
    {
        std::cout << "project config json is not object";
        return false;
    }
    
    conf.build_path = get_string(j["build_path"], "build");
    conf.project_name = get_string(j["project_name"], "UntitledProject");
    conf.source_path = get_string(j["source_path"], "");
    conf.default_state = get_string(j["default_state"], "");

    conf.root_dir = cut_dirpath(path);

    json resources = j["resources"];
    if(resources.is_string())
    {
        load_resources(conf.root_dir + "\\" + resources.get<std::string>(), resources);
    }
    if(!resources.is_object())
    {
        return false;
    }
    conf.resources = resources;

    return true;
}

int main(int argc, char** argv)
{
    LOG("Resource Compiler");
    if(argc < 2)
    {
        LOG("Usage: resource_compiler [project config path]");
        return 0;
    }
    
    project_config pconf;
    if(!load_project_json(argv[1], pconf))
    {
        LOG_ERR("Failed to load project config");
    }

    if(!resource_compiler.Init())
    {
        LOG("Failed to init");
        return 1;
    }

    if(!resource_compiler.Make(pconf))
    {
        LOG_ERR("Failed to make resource archives");
    }
    
    return 0;
}