#ifndef ASSET_TABLE_H
#define ASSET_TABLE_H

#include <mutex>
#include <map>
#include <string>
#include <windows.h>
#include <util/thread_pool.h>
#include "xasset.h"
#include <typeindex.h>

class asset_table
{
public:
    asset_table()
    : _thread_pool(4)
    {
    }

    template<typename T>
    xasset<T> get(const std::string& name)
    {
        auto& map = _assets[TypeInfo<T>::Index()];
        auto it = map.find(name);
        if(it != map.end())
            return *(xasset<T>*)it->second;
        
        xasset<T> a = new xasset<T>();
        xasset a = *(xasset<T>*)map[name];
        _thread_pool.enqueue([this, a](){
            // TODO: load
            add_unsynced(a);
        });
    }

    void sync()
    {
        if(!_sync.try_lock())
            return;
        for(auto a : _unsynced_assets)
        {
            //a.finalize();
        }
        _unsynced_assets.clear();
        _sync.unlock();
    }
private:
    /*
    void add_unsynced(const xasset& a)
    {
        _sync.lock();
        _unsynced_assets.emplace_back(a);
        _sync.unlock();
    }
    */
    std::mutex _sync;
    std::vector<asset_base*> _unsynced_assets;
    std::map<
        typeindex, 
        std::map<std::string, asset_base*>
    > _assets;
    thread_pool _thread_pool;
};

#endif
