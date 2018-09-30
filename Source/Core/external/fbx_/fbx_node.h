#ifndef FBX_NODE_H
#define FBX_NODE_H

#include "fbx_prop.h"
#include <map>

namespace Fbx {

class Node
{
public:
    void AddNode(const Node& node);
    void AddProp(const Property& prop);
    void SetName(const std::string& name);
    const std::string& GetName() const;
    unsigned PropCount() const;
    Property& GetProperty(unsigned i);
    unsigned ChildCount(const std::string& type);
    Node& GetNode(const std::string& type, unsigned i);

    void Print(std::ostringstream& sstr, unsigned level = 0);
private:
    std::string name;
    std::vector<Property> props;
    std::map<std::string, std::vector<Node>> children; 
};

}

#endif
