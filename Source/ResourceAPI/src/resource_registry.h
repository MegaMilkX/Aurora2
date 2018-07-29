#ifndef RESOURCE_REGISTRY_H
#define RESOURCE_REGISTRY_H

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
    void Unload();
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

#endif
