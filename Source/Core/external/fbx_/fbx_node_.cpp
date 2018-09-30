#include "fbx_node.h"
#include "fbx_macro.h"

#include <sstream>

namespace Fbx {

void Node::AddNode(const Node& node){
    children[node.GetName()].emplace_back(node);
}
void Node::AddProp(const Property& prop){
    props.emplace_back(prop);
}
void Node::SetName(const std::string& name){
    this->name = name;
}
const std::string& Node::GetName() const{
    return name;
}
unsigned Node::PropCount() const{
    return props.size();
}
Property& Node::GetProperty(unsigned i){
    return props[i];
}
unsigned Node::ChildCount(const std::string& type) {
    return children[type].size();
}
Node& Node::GetNode(const std::string& type, unsigned i){
    return children[type][i];
}

void Node::Print(std::ostringstream& sstr, unsigned level){
    for(unsigned i = 0; i < level; ++i)
        sstr << "  ";
    sstr << name << " | " << "Prop count: " << props.size() << std::endl;
    ++level;
    for(unsigned i = 0; i < props.size(); ++i)
        props[i].Print(sstr, level);
    for(auto& kv : children)
    {
        for(auto& n : kv.second)
        {
            n.Print(sstr, level);
        }
    }
}

}
