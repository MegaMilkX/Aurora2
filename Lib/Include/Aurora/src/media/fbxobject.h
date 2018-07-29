#ifndef AU_FBXOBJECT_H
#define AU_FBXOBJECT_H

#include "fbxnode.h"

namespace Au{
namespace Media{
namespace FBX{

class Object
{
public:
    virtual ~Object() {}
    
    virtual std::string Name() { return ""; }
private:
};

class Geometry : public Object
{
public:
    std::string Name() { return "Geometry"; }
};

class Deformer : public Object
{
public:
    std::string Name() { return "Deformer"; }
};

class Connection : public Object
{
public:
    std::string Name() { return "Connection"; }
};
 
}}}

#endif
