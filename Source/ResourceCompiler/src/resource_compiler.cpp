#include "resource_compiler_impl.h"

bool ResourceCompilerImpl::Init()
{
    LoadPlugins();

    return true;
}

bool ResourceCompilerImpl::Make(const project_config& conf)
{
    project = conf;

    if(!DeleteExistingArchives(conf))
    {
        LOG_ERR("Failed to delete existing resource archives");
        return false;
    }

    std::string arch_path = conf.root_dir + "\\" + conf.build_path + "\\data\\" + "data.bin";
    memset(&zarch, 0, sizeof(zarch));
    CreateDirectoryA((conf.root_dir + "\\" + conf.build_path + "\\data\\").c_str(), 0);
    if(!mz_zip_writer_init_file(&zarch, arch_path.c_str(), 65537))
    {
        LOG("Failed to create archive " << arch_path);
        return false;
    }

    std::vector<resource> resource_list;
    std::vector<std::string> name_chain;
    json j = conf.resources;
    MakeResourceNode(conf, j, resource_list, name_chain);

    mz_zip_writer_finalize_archive(&zarch);
    mz_zip_writer_end(&zarch);

    return true;
}

bool ResourceCompilerImpl::SubmitMem(const char* name, void* data, size_t len)
{
    if(!mz_zip_writer_add_mem(&zarch, name, data, len, 0))
    {
        LOG_ERR("Failed to SubmitMem() " << name);
        return false;
    }
    return true;
}

bool ResourceCompilerImpl::SubmitFile(const char* name, const char* filename)
{
    if(!mz_zip_writer_add_file(&zarch, name, filename, 0, 0, 0))
    {
        LOG_ERR("Failed to SubmitFile() " << name << ", filename: " << filename);
        return false;
    }
    return true;
}

bool ResourceCompilerImpl::SubmitCopy(const char* filename, const char* filename_relative)
{
    std::string src = filename;
    src = src.substr(src.find_last_of("\\/"));

    std::string to = project.root_dir + "\\" + project.build_path + "\\" + filename_relative;
    if(CopyFileA(filename, to.c_str(), FALSE) != TRUE)
    {
        return false;
    }

    return true;
}

void ResourceCompilerImpl::LoadPlugins()
{
    auto filenames = find_all_files(get_module_dir() + "\\", "*.dll");
    for(auto fname : filenames)
    {
        Plugin* plugin = new Plugin(fname);
        if(!plugin->IsValid())
        {
            delete plugin;
            continue;
        }
        plugins.push_back(plugin);
    }
}

bool ResourceCompilerImpl::DeleteExistingArchives(const project_config& conf)
{
    std::vector<std::string> existing_files = find_all_files(conf.root_dir + "\\" + conf.build_path + "\\data\\", "*");
    for(auto fn : existing_files)
    {
        if(!DeleteFileA(fn.c_str()))
        {
            LOG("Failed to delete " << fn);
            return false;
        }
    }

    return true;
}

void ResourceCompilerImpl::MakeResourceNode(const project_config& conf, json& j, std::vector<resource>& resources, std::vector<std::string>& name_chain)
{
    json jnode_name = j["name"];
    json jnode_source = j["source"];
    json jnode_type = j["type"];
    json jnode_properties = j["properties"];

    bool has_name = false;
    bool has_source = false;
    std::string full_name;
    std::string type;
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
        type = jnode_type.get<std::string>();
        LOG(jnode_type.get<std::string>());
    }
    if(jnode_properties.is_object())
    {
        LOG(jnode_properties.dump());
    }

    if(has_name && has_source)
    {
        std::string source_path = conf.root_dir + "\\" + source_filename;

        bool success = false;
        for(auto p : plugins)
        {
            if(p->MakeResource(this, full_name.c_str(), type.c_str(), source_path.c_str()))
            {
                success = true;
                break;
            }
        }

        if(success)
        {
            LOG(full_name << " compiled");
        }
        else
        {
            LOG_ERR(full_name << " failed to compile");
        }
    }

    json jnodes = j["nodes"];
    if(jnodes.is_array())
    {
        for(auto jnode : jnodes)
        {
            if(!jnode.is_object())
                continue;
            MakeResourceNode(conf, jnode, resources, name_chain);
        }
    }
    
    if(has_name) name_chain.pop_back();
}