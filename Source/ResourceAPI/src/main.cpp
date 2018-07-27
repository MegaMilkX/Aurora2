#include <string>

#include "../../general/util.h"

#define MINIZ_HEADER_FILE_ONLY
#include "../../lib/miniz.c"

#define MKSTR(LINE) \
static_cast<std::ostringstream&>(std::ostringstream() << LINE).str()

#define LOG(LINE) std::cout << MKSTR(LINE) << std::endl;

#include <map>

struct ZipSource
{
    mz_uint32 index;
    mz_uint64 size;
    mz_zip_archive* archive;
};

class Resource
{
public:
    Resource();
    Resource(const ZipSource& source);
    bool Load();
    Resource* Get(const std::string& local_name);

    template<typename T>
    bool Build(T& obj);

    unsigned char* Data()
    {
        return buffer.data();
    }
    size_t DataSize()
    {
        return buffer.size();
    }

    void _addChild(const std::string& local_name, Resource* r);
private:
    ZipSource source;
    bool loaded = false;
    std::map<std::string, Resource*> children;
    std::vector<unsigned char> buffer;
};

class ResourceRegistry
{
public:
    ~ResourceRegistry();

    void Init();

    void Cleanup();

    void ListAll();
    
    bool Exists(const std::string& name);

    Resource* Get(const std::string& name);
private:
    Resource* CreateResourceRecursive(const std::string& name, const ZipSource& source);
    Resource* GetOrCreate(const std::string& name, const ZipSource& source);

    std::map<std::string, Resource*> resources;
    std::map<std::string, mz_zip_archive*> archives;
};

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
        buffer.resize(source.size);
        mz_zip_reader_extract_to_mem(source.archive, source.index, (void*)buffer.data(), source.size, 0);
    }

    loaded = true;
    return true;
}

Resource* Resource::Get(const std::string& local_name)
{
    if(children.count(local_name) == 0) return 0;
    return children[local_name];
}

template<typename T>
bool Resource::Build(T& obj) 
{ 
    return obj.Build(this);
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

            LOG(file_stat.m_filename);
            
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

class Text
{
public:
    bool Build(Resource* r)
    {
        data = std::string((char*)r->Data(), (char*)r->Data() + r->DataSize());
        return true;
    }

    void Print()
    {
        LOG(data);
    }
private:
    std::string data;
};

int main()
{
    ResourceRegistry registry;
    registry.Init();
    
    Resource* res = registry.Get("General.Player.Anim");
    Resource* res_idle = res->Get("Idle");

    if(!registry.Get("General")->Load())
    {
        LOG("Failed to load General resource");
    }
    LOG("General loaded");

    Resource* r = registry.Get("General.tex");
    Text t;
    r->Build(t);
    t.Print();

    return 0;
}

#undef MINIZ_HEADER_FILE_ONLY
#include "../../lib/miniz.c"