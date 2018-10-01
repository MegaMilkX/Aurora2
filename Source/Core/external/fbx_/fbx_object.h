#ifndef FBX_OBJECT_2_H
#define FBX_OBJECT_2_H

#include <stdint.h>
#include <string>
#include "fbx_node.h"
#include "fbx_math.h"

namespace Fbx {

class Node;
class Object
{
public:
    virtual ~Object() {}
    //void SetUid(int64_t uid) { this->uid = uid; }
    //int64_t GetUid() const { return uid; }
    //void SetName(const std::string& str) { name = str; }
    //const std::string& GetName() const { return name; }

    virtual bool Make(Node& node) = 0;
protected:
    //int64_t uid;
    //std::string name;
};

}

#endif
