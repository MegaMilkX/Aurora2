#ifndef FBX_PROPERTIES_2_H
#define FBX_PROPERTIES_2_H

#include "fbx_object.h"
#include "fbx_log.h"
#include "fbx_macro.h"
#include "fbx_type_index.h"
#include "fbx_math.h"

#include <memory>

namespace Fbx {

class ValueHolderBase {
public:
    virtual ~ValueHolderBase() {}
    template<typename Y>
    Y Get() {
        if(GetType() != TypeInfo<Y>::Index())
            return Y();
        Y v = *(Y*)GetPtr();
        return v;
    }
protected:
    virtual TypeIndex GetType() = 0;
    virtual void* GetPtr() = 0;
};

template<typename T>
class ValueHolder : public ValueHolderBase {
public:
    ValueHolder(const T& v)
    : value(v) {}
protected:
    virtual TypeIndex GetType() { return TypeInfo<T>::Index(); }
    virtual void* GetPtr() { return (void*)&value; }
private:
    T value;
};

class Variant {
public:
    Variant()
    : type(TypeInfo<void>::Index()) {}
    ~Variant() {}
    template<typename T>
    void Set(const T& v) {
        type = TypeInfo<T>::Index();
        value.reset(new ValueHolder<T>(v));
    }
    template<typename T>
    bool Is() {
        if(type != TypeInfo<T>::Index())
            return false;
        return true;
    }
    template<typename T>
    T Get() {
        if(type != TypeInfo<T>::Index())
            return T();
        return value->Get<T>();
    }
private:
    TypeIndex type;
    std::shared_ptr<ValueHolderBase> value;
};

class Property : public BasicObject {
public:
    
    virtual bool Make(Node& node) {
        name = node.GetProperty(0).GetString();
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
                variant.Set(p.GetDouble());
            } else if(p.IsInt16()) {
                variant.Set(p.GetInt16());
            } else if(p.IsInt32()) {
                variant.Set(p.GetInt32());
            } else if(p.IsInt64()) {
                variant.Set(p.GetInt64());
            } else if(p.IsFloat()) {
                variant.Set(p.GetFloat());
            } else if(p.IsString()) {
                variant.Set(p.GetString());
            }
        } else if(node.PropCount() == 6) {
            NodeProperty& p0 = node.GetProperty(4);
            NodeProperty& p1 = node.GetProperty(5);
            if(p0.IsDouble() && p1.IsDouble()) {
                variant.Set(
                    FbxDVector2(
                        p0.GetDouble(), 
                        p1.GetDouble()
                    )
                );
            } else if(p0.IsInt16() && p1.IsInt16()) {
                variant.Set(
                    FbxIVector2(
                        p0.GetInt16(),
                        p1.GetInt16()
                    )
                );
            } else if(p0.IsInt32() && p1.IsInt32()) {
                variant.Set(
                    FbxIVector2(
                        p0.GetInt32(),
                        p1.GetInt32()
                    )
                );
            } else if(p0.IsInt64() && p1.IsInt64()) {
                variant.Set(
                    FbxIVector2(
                        p0.GetInt64(),
                        p1.GetInt64()
                    )
                );
            } else if(p0.IsFloat() && p1.IsFloat()) {
                variant.Set(
                    FbxVector2(
                        p0.GetFloat(),
                        p1.GetFloat()
                    )
                );
            }
        } else if(node.PropCount() == 7) {
            NodeProperty& p0 = node.GetProperty(4);
            NodeProperty& p1 = node.GetProperty(5);
            NodeProperty& p2 = node.GetProperty(6);
            if(p0.IsDouble() && p1.IsDouble() && p2.IsDouble()) {
                variant.Set(
                    FbxDVector3(
                        p0.GetDouble(), 
                        p1.GetDouble(), 
                        p2.GetDouble()
                    )
                );
            } else if(p0.IsInt16() && p1.IsInt16() && p2.IsInt16()) {
                variant.Set(
                    FbxIVector3(
                        p0.GetInt16(),
                        p1.GetInt16(),
                        p2.GetInt16()
                    )
                );
            } else if(p0.IsInt32() && p1.IsInt32() && p2.IsInt32()) {
                variant.Set(
                    FbxIVector3(
                        p0.GetInt32(),
                        p1.GetInt32(),
                        p2.GetInt32()
                    )
                );
            } else if(p0.IsInt64() && p1.IsInt64() && p2.IsInt64()) {
                variant.Set(
                    FbxIVector3(
                        p0.GetInt64(),
                        p1.GetInt64(),
                        p2.GetInt64()
                    )
                );
            } else if(p0.IsFloat() && p1.IsFloat() && p2.IsFloat()) {
                variant.Set(
                    FbxVector3(
                        p0.GetFloat(),
                        p1.GetFloat(),
                        p2.GetFloat()
                    )
                );
            }
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

    template<typename T>
    T Get() {
        if(!variant.Is<T>()) {
            FBX_LOGW("Property '" << name "' - invalid type specified when getting value");
        }
        return variant.Get<T>();
    }

    Variant& GetVariant() {
        return variant;
    }
private:
    std::string name;
    Variant variant;
};

class Properties : public BasicObject {
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
    template<typename T>
    T GetValue(const std::string& name) {
        T v = T();
        Property* p = Get(name);
        if(p) {
            v = p->GetVariant().Get<T>();
        } else {
            FBX_LOGW(name << " - no such property");
        }
        return v;
    }
private:
    std::vector<Property*> props;
};

}

#endif