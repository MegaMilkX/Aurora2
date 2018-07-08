#ifndef ASSET_H
#define ASSET_H

#include <functional>
#include <util/load_asset.h>
#include "resources/resource.h"

template<typename T>
class asset
{
public:
    class reader
    {
    friend asset;
    public:
        virtual ~reader() {}
        virtual bool operator()(const std::string& filename, T*) = 0;
    private:
        std::string extension;
    };

    asset()
    : data(0) 
    {}
    asset(resource<T> data)
    : data(data)
    {}
    static void add_search_path(const std::string& path)
    { searchPaths.push_back(path); }
    /*
    template<typename READER>
    static void add_reader(const std::string& extension)
    {
        READER* rdr = new READER();
        rdr->extension = extension;
        readers.push_back(rdr);
    }
    */
    static void add_reader(const std::string& extension, std::function<bool(T*, const std::string&)> func)
    {
        extensions.push_back(extension);
        readers.push_back(func);
    }
    
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
            for(unsigned i = 0; i < searchPaths.size(); ++i)
            {
                for(unsigned j = 0; j < extensions.size(); ++j)
                {
                    // TODO: REMOVE PLATFORM DEPENDENT CODE
                    // TODO: RESPECT EMPTY SEARCH PATHS
                    // TODO: MAKE FULL FILE PATH
                    filename = 
                        searchPaths[i] + 
                        "\\" + 
                        name +
                        "." +
                        extensions[j];
                    
                    if(readers[j](r, filename))
                    {
                        return asset<T>(r);
                    }
                }
            }
            std::cout << "Asset not found: " << filename << std::endl;
            return asset<T>(r);
        }
        
    }
private:
    resource<T> data;
    static std::vector<std::string> searchPaths;
    static std::vector<std::function<bool(T*, const std::string&)>> readers;
    static std::vector<std::string> extensions;
};

template<typename T>
std::vector<std::string> asset<T>::searchPaths;
template<typename T>
std::vector<std::function<bool(T*, const std::string&)>> asset<T>::readers;
template<typename T>
std::vector<std::string> asset<T>::extensions;

#endif
