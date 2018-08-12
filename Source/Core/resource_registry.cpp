#include "resource_registry.h"

ResourceRegistry g_resourceRegistry;

Resource::Resource()
: source{0, 0}
{

}

Resource::Resource(const ZipSource& source)
: source(source)
{

}

bool Resource::Load()
{
    if(loaded) return true;

    for(auto kv : children)
    {
        kv.second->Load();
    }

    if(source.archive != 0)
    {
        buffer.resize((size_t)source.size);
        mz_zip_reader_extract_to_mem(source.archive, source.index, (void*)buffer.data(), (size_t)source.size, 0);
    }

    loaded = true;
    return true;
}

void Resource::Unload()
{
    buffer.clear();
    loaded = false;
}

Resource* Resource::Get(const std::string& local_name)
{
    if(children.count(local_name) == 0) return 0;
    return children[local_name];
}

void Resource::_addChild(const std::string& local_name, Resource* r)
{
    children[local_name] = r;
}

ResourceRegistry::~ResourceRegistry()
{
    Cleanup();
}

void ResourceRegistry::Init()
{
    Cleanup();

    std::vector<std::string> filenames = find_all_files(get_module_dir() + "\\data", "*.bin");
    for(auto fname : filenames)
    {
        mz_zip_archive* zip = new mz_zip_archive();
        memset(zip, 0, sizeof(*zip));
        if(!mz_zip_reader_init_file(zip, fname.c_str(), 0))
        {
            LOG("Failed to open " << fname);
            continue;
        }

        mz_uint num_files = mz_zip_reader_get_num_files(zip);
        LOG(fname << " has " << num_files << " entries");
        
        archives.insert(
            std::make_pair(
                std::string(fname),
                zip
            )
        );

        for(unsigned i = 0; i < num_files; ++i)
        {
            mz_zip_archive_file_stat file_stat;
            if(!mz_zip_reader_file_stat(zip, i, &file_stat))
            {
                LOG("Failed to get file stat for file at index " << i);
                continue;
            }

            //LOG(file_stat.m_filename);
            
            Resource* r = CreateResourceRecursive(file_stat.m_filename, ZipSource{file_stat.m_file_index, file_stat.m_uncomp_size, zip});
        }
    }
}

void ResourceRegistry::Cleanup()
{
    for(auto kv : archives)
    {
        mz_zip_reader_end(kv.second);
        delete kv.second;
    }
    archives.clear();

    for(auto kv : resources)
    {
        delete kv.second;
    }
    resources.clear();
}

void ResourceRegistry::ListAll()
{
    for(auto kv : resources)
    {
        LOG(kv.first);
    }
}

bool ResourceRegistry::Exists(const std::string& name)
{
    return resources.count(name) != 0;
}

Resource* ResourceRegistry::Get(const std::string& name)
{
    if(!Exists(name))
        return 0;
    return resources[name];
}

Resource* ResourceRegistry::CreateResourceRecursive(const std::string& name, const ZipSource& source)
{
    std::vector<std::string> tokens = split(name, '.');
    Resource* parent = 0;
    std::string new_name = "";
    for(unsigned i = 0; i < tokens.size(); ++i)
    {
        std::string t = tokens[i];

        if(!new_name.empty()) new_name += ".";
        new_name += t;

        Resource* res;
        if(i < tokens.size() - 1)
            res = GetOrCreate(new_name, ZipSource{0, 0, 0});
        else
            res = GetOrCreate(new_name, source);

        if(parent) parent->_addChild(t, res);
        parent = res;
    }

    return parent;
}

Resource* ResourceRegistry::GetOrCreate(const std::string& name, const ZipSource& source)
{
    Resource* r = 0;
    if(resources.count(name) == 0)
    {
        r = new Resource(source);
        resources.insert(std::make_pair(name, r));
    }
    else
    {
        r = resources[name];
    }
    return r;
}
