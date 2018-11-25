#include "fbx_node.h"
#include "fbx_macro.h"
#include "fbx_object.h"

#include <sstream>

namespace Fbx {

Node::~Node() {
    for(auto& kv : object_cache) {
        delete kv.second;
    }
}
void Node::AddNode(const Node& node){
    children.emplace_back(node);
}
void Node::AddProp(const NodeProperty& prop){
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
NodeProperty& Node::GetProperty(unsigned i){
    return props[i];
}
size_t Node::ChildCount() {
    return children.size();
}
Node& Node::GetNode(unsigned i){
    return children[i];
}
Node* Node::FindNode(const std::string& name) {
    return FindNode(name, 0);
}
Node* Node::FindNode(const std::string& name, unsigned i) {
    unsigned counter = 0;
    for(size_t j = 0; j < ChildCount(); ++j) {
        Node& n = GetNode(j);
        if(n.GetName() == name) {
            if(counter == i) {
                return &n;
            }
            counter++;
        }
    }
    return 0;
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

void Node::SetUidAndName(Node& node, Object* o) {
    if(node.PropCount() < 2) return;
    if(node.GetProperty(0).IsInt64())
        o->SetUid(node.GetProperty(0).GetInt64());
    if(node.GetProperty(1).IsString())
        o->SetName(node.GetProperty(1).GetString());
}
void Node::SetUidAndName(Node& node, BasicObject* o) {
    // Do nothing
}

}
