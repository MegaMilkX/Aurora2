#ifndef FBX_PROPERTIES_2_H
#define FBX_PROPERTIES_2_H

#include "fbx_object.h"
#include "fbx_log.h"
#include "fbx_macro.h"
#include "fbx_type_index.h"
#include "fbx_math.h"

namespace Fbx {

class Property : public Object {
public:
    virtual bool Make(Node& node) {
        name = node.GetProperty(0).GetString();
        std::string type_str = node.GetProperty(1).GetString();
        /*
        { "bool",
            "Number",
            "KString",
            "DateTime",
            "Compound",
            "int",
            "double",
            "KTime",
            "enum",
            "ColorRGB",
            "Color",
            "Vector",
            "Vector3D" };*/
        if(node.PropCount() == 5) {
            NodeProperty& p = node.GetProperty(4);
            if(p.IsDouble()) {
                type = TypeInfo<double>::Index();
                double v = p.GetDouble();
            } else if(p.IsInt16()) {
                type = TypeInfo<int16_t>::Index();
            } else if(p.IsInt32()) {
                type = TypeInfo<int32_t>::Index();
            } else if(p.IsInt64()) {
                type = TypeInfo<int64_t>::Index();
            } else if(p.IsFloat()) {
                type = TypeInfo<float>::Index();
            } else if(p.IsString()) {
                type = TypeInfo<std::string>::Index();
            }
        } else if(node.PropCount() == 6) {
            NodeProperty& p0 = node.GetProperty(4);
            NodeProperty& p1 = node.GetProperty(5);

            type = TypeInfo<void>::Index();
        } else if(node.PropCount() == 7) {
            NodeProperty& p0 = node.GetProperty(4);
            NodeProperty& p1 = node.GetProperty(5);
            NodeProperty& p2 = node.GetProperty(6);
            if(p0.IsDouble() && p1.IsDouble() && p2.IsDouble()) {
                // TODO
            } else if(p0.IsInt16() && p1.IsInt16() && p2.IsInt16()) {
                //type = TypeInfo<int16_t>::Index();
            } else if(p0.IsInt32() && p1.IsInt32() && p2.IsInt32()) {
                //type = TypeInfo<int32_t>::Index();
            } else if(p0.IsInt64() && p1.IsInt64() && p2.IsInt64()) {
                //type = TypeInfo<int64_t>::Index();
            } else if(p0.IsFloat() && p1.IsFloat() && p2.IsFloat()) {
                //type = TypeInfo<float>::Index();
            }
        } else {
            type = TypeInfo<void>::Index();
        }
        return true;
    }

    static bool IdentifyNode(Node& node) {
        if(node.GetName() != "P"
            || node.PropCount() < 5
            || !node.GetProperty(0).IsString()
            || !node.GetProperty(1).IsString()
            || !node.GetProperty(2).IsString())
            return false;
        return true;
    }

    const std::string& Name() const {
        return name;
    }
private:
    std::string name;
    TypeIndex type;
};

class Properties : public Object {
public:
    virtual bool Make(Node& node) {
        for(size_t i = 0; i < node.ChildCount(); ++i) {
            Node& n = node.GetNode(i);
            Property* prop = n.Convert<Property>();
            if(prop) props.emplace_back(prop);
        }
        return true;
    }
    static bool IdentifyNode(Node& node) {
        if(node.GetName() != "Properties70") 
            return false;
        return true;
    }

    Property* Get(const std::string& name) {
        for(size_t i = 0; i < props.size(); ++i) {
            Property* p = props[i];
            if(p->Name() == name) return p;   
        }
        return 0;
    }
private:
    std::vector<Property*> props;
};

}

#endif