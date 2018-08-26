#ifndef FBX_OBJECT_H
#define FBX_OBJECT_H

#include <stdint.h>

class FbxObject
{
public:
    virtual ~FbxObject() {}
    void SetUid(int64_t uid) { this->uid = uid; }
    int64_t GetUid() const { return uid; }
protected:
    int64_t uid;
};

#endif
