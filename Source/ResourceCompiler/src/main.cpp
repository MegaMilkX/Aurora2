#include "../../general/util.h"

#define MKSTR(LINE) \
static_cast<std::ostringstream&>(std::ostringstream() << LINE).str()

#define LOG(LINE) std::cout << MKSTR(LINE) << std::endl;

using json = nlohmann::json;

struct project_config
{
    std::string build_path;
    std::string project_name;
    std::string source_path;
    std::string default_state;

    std::string root_dir;

    json resources;
};

struct resource
{
    std::string full_name;
    std::string source_file;
    std::string type;
    json properties;
};

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

void process_resource_json_node(json& j, std::vector<resource>& resources, std::vector<std::string>& name_chain)
{
    json jnode_name = j["name"];
    json jnode_source = j["source"];
    json jnode_type = j["type"];
    json jnode_properties = j["properties"];

    bool has_name = false;
    if(jnode_name.is_string())
    {
        name_chain.push_back(jnode_name.get<std::string>());
        
        std::string full_name;
        for(std::string part : name_chain)
        {
            if(!full_name.empty())
                full_name += ".";
            full_name += part;
        }
        LOG(full_name);
        has_name = true;
    }
    if(jnode_source.is_string())
    {
        LOG(jnode_source.get<std::string>());
    }
    if(jnode_type.is_string())
    {
        LOG(jnode_type.get<std::string>());
    }
    if(jnode_properties.is_object())
    {
        LOG(jnode_properties.dump());
    }

    json jnodes = j["nodes"];
    if(jnodes.is_array())
    {
        for(auto jnode : jnodes)
        {
            if(!jnode.is_object())
                continue;
            process_resource_json_node(jnode, resources, name_chain);
        }
    }
    
    if(has_name) name_chain.pop_back();
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

    std::vector<resource> resource_list;
    std::vector<std::string> name_chain;
    process_resource_json_node(resources, resource_list, name_chain);

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
    load_project_json(argv[1], pconf);
    std::string build_dir = pconf.root_dir + "\\" + pconf.build_path;
    LOG(build_dir);
    
    std::getchar();
    return 0;
}