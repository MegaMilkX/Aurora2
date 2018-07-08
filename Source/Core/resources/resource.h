#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <string>
#include <map>
#include <vector>

template<typename T>
class resource
{
public:
    // Non-static
    resource(T* data = 0)
    : data(data), name(name) {}
    resource(const std::string& name, T* data)
    : data(data), name(name) {}
    ~resource() { decrease_count(name); }
    T* operator->() const { return data; }
    operator T*() const { return data; }
    operator bool() const { return data != 0; }
    T& operator *() const { return *data; }

    bool equals(const resource<T>& other) const { return data == other.data; }

    // Static 

    static bool exists(const std::string& name)
    {
        return resources.count(name) != 0;
    }

    static resource<T> get(const std::string& name)
    {
        T* res = 0;
        std::map<std::string, T*>::iterator it = 
            resources.find(name);
        if(it != resources.end())
        {
            referenceCount[name]++;
            return it->second;
        }
        else
        {
            T* r = new T;
            resources[name] = r;
            referenceCount[name]++;
            return r;
        }
    }
    
    static void free(const std::string& name)
    {
        if(referenceCount[name] > 0)
            return;
        
        std::map<std::string, T*>::iterator it =
            resources.find(name);
        if(it != resources.end())
        {
            delete it->second;
            resources.erase(name);
        }
    }
    
private:
    static void decrease_count(const std::string& name)
    {
        referenceCount[name]--;
    }
    T* data;
    std::string name;
    static std::map<std::string, T*> resources;
    static std::map<std::string, int> referenceCount;
};

template<typename T>
std::map<std::string, T*> resource<T>::resources;
template<typename T>
std::map<std::string, int> resource<T>::referenceCount;

#endif
