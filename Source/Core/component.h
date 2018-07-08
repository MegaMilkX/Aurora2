#ifndef COMPONENT_H
#define COMPONENT_H

#include <map>
#include <string>
#include <iostream>
#include <typeindex.h>

typedef std::map<std::string, int> component_map_t;
typedef void*(*alloc_fn_t)(void);
typedef void(*free_fn_t)(void*);

template<typename T>
void* meta_alloc_fn() {
    return (void*)new T();
}
template<typename T>
void meta_free_fn(void* ptr) {
    delete ((T*)ptr);
}

class MetaData
{
public:
    MetaData()
    : alloc(0), free(0), size(0)
    {

    }
    MetaData(const std::string& name, size_t size, alloc_fn_t alloc, free_fn_t free, typeindex tid)
    : name(name), size(size), alloc(alloc), free(free), type_index(tid)
    {

    }

    std::string name;
    size_t size;
    alloc_fn_t alloc;
    free_fn_t free;
    typeindex type_index;
};

class Meta
{
public:
    Meta()
    : metaData() {}
    Meta(MetaData data)
    : metaData(data) {}

    static Meta Get(const std::string& name)
    {
        auto& map = GetMetaMap();
        auto it = map.find(name);
        if(it == map.end())
            return Meta();
        return it->second;
    }

    typeindex TypeIndex() const { return metaData.type_index; }
    std::string Name() const { return metaData.name; }

    void* Create()
    {
        if(metaData.alloc == 0)
            return 0;
        return metaData.alloc();
    }

    template<typename T>
    static void Init(const std::string& name)
    {
        MetaData md = {
            name,
            sizeof(T),
            &meta_alloc_fn<T>,
            &meta_free_fn<T>,
            TypeInfo<T>::Index()
        };
        GetMetaMap()[name] = Meta(md);
    }
    
    static void Print()
    {
        for(auto& kv : GetMetaMap())
        {
            std::cout << kv.second.metaData.name << std::endl;
        }
    }
private:
    static std::map<std::string, Meta>& GetMetaMap()
    {
        static std::map<std::string, Meta> _meta;
        return _meta; 
    }
    MetaData metaData;
};

#define COMPONENT(NAME) \
template<typename T> \
struct NAME ## _dummyWrap { \
    static int dummy; \
}; \
inline int NAME ## _dummyInit() { \
    NAME ## _dummyWrap<int>::dummy = 0; \
    Meta::Init<NAME>(#NAME); \
    return 0; \
} \
template<typename T> \
int NAME ## _dummyWrap<T>::dummy = NAME ## _dummyInit();

void InitComponentMeta();
component_map_t& ComponentMetaMap(bool update = false, const std::string& name = "", int value = 0);


#endif
