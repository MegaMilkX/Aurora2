#ifndef SCENE_EXPORT_DATA_H
#define SCENE_EXPORT_DATA_H

#include <scene_object.h>
#include <resource.h>

class ExportData {
public:
    void Clear() {
        resources.clear();
    }

    template<typename T>
    void AddResource(std::shared_ptr<T> res) {
        resources.insert(res);
    }

    std::set<std::shared_ptr<Resource>>& GetResources() {
        return resources;
    }
private:
    std::set<std::shared_ptr<Resource>> resources;
};

#endif
