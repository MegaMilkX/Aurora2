#ifndef RESOURCE_H
#define RESOURCE_H

#include "data_source.h"
#include <rttr/type>

class Resource {
    RTTR_ENABLE()
public:
    enum STORAGE
    {
        GLOBAL,
        LOCAL
    };

    virtual ~Resource() {}
    virtual bool Build(DataSourceRef src) = 0;
    virtual bool Serialize(std::vector<unsigned char>& data) = 0;
    const std::string& Name() const { return name; }
    void Name(const std::string& name) { this->name = name; }
    STORAGE Storage() const { return storage; }
    void Storage(STORAGE storage) { this->storage = storage; }
private:
    std::string name;
    STORAGE storage;
};

#endif
