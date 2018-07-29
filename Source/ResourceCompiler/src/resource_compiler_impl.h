#ifndef RESOURCE_COMPILER_IMPL
#define RESOURCE_COMPILER_IMPL

#include "resource_compiler.h"

#include "../../general/util.h"

#define MINIZ_HEADER_FILE_ONLY
#include "miniz.c"

class Plugin
{
public:
    typedef bool(*MakeResource_fn_t)(ResourceCompiler*, const char*, const char*, const char*);

    Plugin(const std::string& filename)
    : module(0), MakeResource(0)
    {
        module = LoadLibraryA(filename.c_str());
        if(module == NULL)
        {
            LOG_WARN("Failed to load " << filename);
        }

        MakeResource = (MakeResource_fn_t)GetProcAddress(module, "MakeResource");
        if(MakeResource == NULL)
        {
            LOG_WARN(filename << " is not a valid plugin");
        }
    }
    ~Plugin()
    {
        if(module) FreeLibrary(module);
    }
    bool IsValid() 
    { 
        return (MakeResource && module); 
    }
    MakeResource_fn_t MakeResource;
private:
    HMODULE module;
};

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

class ResourceCompilerImpl : public ResourceCompiler
{
public:
    bool Init();

    bool Make(const project_config& conf);
private:
    virtual bool SubmitMem(const char* name, void* data, size_t len);
    virtual bool SubmitFile(const char* name, const char* filename);

    void LoadPlugins();
    bool DeleteExistingArchives(const project_config& conf);
    void MakeResourceNode(const project_config& conf, json& j, std::vector<resource>& resources, std::vector<std::string>& name_chain);

    mz_zip_archive zarch;
    std::vector<Plugin*> plugins;
};

#endif
