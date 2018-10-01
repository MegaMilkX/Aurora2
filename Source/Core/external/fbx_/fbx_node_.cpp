#include "fbx_node.h"
#include "fbx_macro.h"

#include <sstream>

namespace Fbx {

void Node::AddNode(const Node& node){
    children.emplace_back(node);
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
size_t Node::ChildCount() {
    return children.size();
}
Node& Node::GetNode(unsigned i){
    return children[i];
}

void Node::Print(std::ostringstream& sstr, unsigned level){
    for(unsigned i = 0; i < level; ++i)
        sstr << "  ";
    sstr << name << " | " << "Prop count: " << props.size() << std::endl;
    ++level;
    for(unsigned i = 0; i < props.size(); ++i)
        props[i].Print(sstr, level);
    for(auto& n : children)
    {
        n.Print(sstr, level);
    }
}

}
