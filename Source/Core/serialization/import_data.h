#ifndef IMPORT_DATA_H
#define IMPORT_DATA_H

#include <scene_object.h>
#include <resource.h>

class ImportData {
public:
    void Clear() {
        oldUidToObject.clear();
    }

    void AddObject(int64_t imported_uid, SceneObject* so) {
        oldUidToObject[imported_uid] = so;
    }
    SceneObject* GetObjectByImportUid(int64_t uid) {
        auto it = oldUidToObject.find(uid);
        if(it == oldUidToObject.end()) return 0;
        return it->second;
    }

    template<typename T>
    std::shared_ptr<T> GetResource(const std::string& name) {
        auto res_it = resources.find(name);
        if(res_it != resources.end()) {
            return *(std::shared_ptr<T>*)&res_it->second;
        } else {
            std::cout << "Searching for " << name << " local data source" << std::endl;
            if(data_sources.count(name) == 0) {
                std::cout << "Local resource " << name << " doesn't exist" << std::endl;
                return std::shared_ptr<T>();
            }
            std::shared_ptr<T> res(new T());
            if(!res->Build(data_sources[name])) {
                std::cout << "Local resource import: build failed" << std::endl;
            }
            res->Name(name);
            res->Storage(Resource::LOCAL);
            resources[name] = res;
            return res;
        }
    }

    std::map<std::string, DataSourceRef> data_sources;
    std::vector<std::function<void()>> deferred_tasks;
private:
    std::map<std::string, std::shared_ptr<Resource>> resources;
    std::map<int64_t, SceneObject*> oldUidToObject;
};

#endif
