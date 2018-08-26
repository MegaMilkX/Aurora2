#ifndef FBX_NODE_H
#define FBX_NODE_H

#include "fbx_prop.h"
#include <map>

#define PROPERTY_NODE_NAME "Properties70"

class FbxNode
{
public:
    void AddNode(const FbxNode& node);
    void AddProp(const FbxProp& prop);
    void SetName(const std::string& name);
    const std::string& GetName() const;
    unsigned PropCount() const;
    FbxProp& GetProperty(unsigned i);
    unsigned ChildCount(const std::string& type);
    FbxNode& GetNode(const std::string& type, unsigned i);

    void Print(std::ostringstream& sstr, unsigned level = 0);
private:
    std::string name;
    std::vector<FbxProp> props;
    std::map<std::string, std::vector<FbxNode>> children; 
};

#endif
