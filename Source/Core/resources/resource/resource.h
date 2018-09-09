#ifndef RESOURCE_H
#define RESOURCE_H

#include "data_source.h"

class Resource {
public:
    virtual ~Resource() {}
    virtual bool Build(DataSourceRef src) = 0;
    const std::string& Name() const { return name; }
    void Name(const std::string& name) { this->name = name; }
private:
    std::string name;
};

#endif
