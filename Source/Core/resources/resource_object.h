#ifndef RESOURCE_OBJECT_H
#define RESOURCE_OBJECT_H

#include "resource_registry.h"

class ResourceObject
{
public:
    virtual ~ResourceObject() {}
    virtual bool Build(ResourceRaw* raw) = 0;
};

#endif
