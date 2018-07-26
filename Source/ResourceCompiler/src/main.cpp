#include "../../general/util.h"

#define MINIZ_HEADER_FILE_ONLY
#include "miniz.c"

#define MKSTR(LINE) \
static_cast<std::ostringstream&>(std::ostringstream() << LINE).str()

#define LOG(LINE) std::cout << MKSTR(LINE) << std::endl;

using json = nlohmann::json;

std::vector<std::string> find_all_files(const std::string& dir, const std::string& filter)
{
    std::string full_filter = dir + "\\" + filter;
    std::vector<std::string> names;
    WIN32_FIND_DATAA data;
    HANDLE hFind = FindFirstFileA(full_filter.c_str(), &data);

    char buf[260];
    DWORD len = GetFullPathNameA(full_filter.c_str(), 260, buf, 0);
    std::string dirpath(buf, len);

    if ( hFind != INVALID_HANDLE_VALUE ) 
    {
        do 
        {
            names.push_back(dir + "\\" + std::string(data.cFileName));
        } while (FindNextFileA(hFind, &data));
        FindClose(hFind);
    }

    return names;
}

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

mz_zip_archive zarch;

void process_resource_json_node(project_config& conf, json& j, std::vector<resource>& resources, std::vector<std::string>& name_chain)
{
    json jnode_name = j["name"];
    json jnode_source = j["source"];
    json jnode_type = j["type"];
    json jnode_properties = j["properties"];

    bool has_name = false;
    bool has_source = false;
    std::string full_name;
    std::string source_filename;
    if(jnode_name.is_string())
    {
        name_chain.push_back(jnode_name.get<std::string>());
        
        
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
        has_source = true;
        source_filename = jnode_source.get<std::string>();
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

    if(has_name && has_source)
    {
        std::string source_path = conf.root_dir + "\\" + source_filename;
        if(!mz_zip_writer_add_file(&zarch, full_name.c_str(), source_path.c_str(), "", 0, 0))
        {
            LOG("Failed to archive target " << source_path);
        }
    }

    json jnodes = j["nodes"];
    if(jnodes.is_array())
    {
        for(auto jnode : jnodes)
        {
            if(!jnode.is_object())
                continue;
            process_resource_json_node(conf, jnode, resources, name_chain);
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

    std::vector<std::string> existing_files = find_all_files(conf.root_dir + "\\" + conf.build_path + "\\data\\", "*");
    for(auto fn : existing_files)
    {
        if(!DeleteFileA(fn.c_str()))
        {
            LOG("Failed to delete " << fn);
        }
    }

    std::string arch_path = conf.root_dir + "\\" + conf.build_path + "\\data\\" + "data_0.bin";
    memset(&zarch, 0, sizeof(zarch));
    CreateDirectoryA((conf.root_dir + "\\" + conf.build_path + "\\data\\").c_str(), 0);
    if(!mz_zip_writer_init_file(&zarch, arch_path.c_str(), 65537))
    {
        LOG("Failed to create archive " << arch_path);
    }

    std::vector<resource> resource_list;
    std::vector<std::string> name_chain;
    process_resource_json_node(conf, resources, resource_list, name_chain);

    mz_zip_writer_end(&zarch);

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