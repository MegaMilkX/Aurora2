#ifndef FBX_OBJECT_2_H
#define FBX_OBJECT_2_H

#include <stdint.h>
#include <string>
#include "fbx_node.h"

namespace Fbx {

class Object
{
public:
    virtual ~Object() {}
    void SetUid(int64_t uid) { this->uid = uid; }
    int64_t GetUid() const { return uid; }

    virtual bool Make(Node& node) = 0;
protected:
    int64_t uid;
    std::string name;
};

}

#endif
