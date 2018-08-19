#ifndef ASSET_H
#define ASSET_H

#include <functional>
#include <util/load_asset.h>
#include "resources/resource.h"

#include "resource_registry.h"

template<typename T>
class asset
{
public:
    asset()
    : data(0) 
    {}
    asset(resource<T> data)
    : data(data)
    {}
    
    void set(const std::string& name)
    {
        *this = get(name);
    }
    void operator=(const resource<T> other)
    {
        this->data = other;
    }

    T* operator->() const { return data.operator->(); }
    operator bool() const { return data != 0; }
    operator T*() const { return data; }
    bool empty() { return data == 0; }

    static asset<T> get(const std::string& name) 
    {
        std::string filename;
        if(resource<T>::exists(name))
            return asset<T>(resource<T>::get(name));
        else
        {
            resource<T> r = resource<T>::get(name);
            /*
            Resource* raw_r = g_resourceRegistry.Get(name);
            if(raw_r == 0)
            {
                std::cout << "Asset not found: " << name << std::endl;
                return asset<T>(r);
            }
            if(!raw_r->Load())
            {
                std::cout << "Resource load failed: " << name << std::endl;
                return asset<T>(r);
            }
            if(!raw_r->Build<T>(*r))
            {
                std::cout << "Failed to build asset: " << name << std::endl;
                return asset<T>(r);
            }
            */
            return asset<T>(r);
        }
        
    }
private:
    resource<T> data;
};

#endif
