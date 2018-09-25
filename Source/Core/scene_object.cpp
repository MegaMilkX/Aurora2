#include "scene_object.h"
#include "scene_controller.h"

SceneObject::~SceneObject()
{
    //for(unsigned i = 0; i < objects.size(); ++i)
    //    delete objects[i];
    for(auto& kv : components)
    {
        if(controller)
        {
            controller->_onRemoveComponent(kv.first, kv.second, this);
        }
        delete kv.second;
    }
}

SceneObject* SceneObject::Root() {
    if(!parentObject)
        return this;
    else
        return parentObject->Root();
}

SceneObject* SceneObject::Parent() {
    return parentObject;
}

bool SceneObject::IsRoot() {
    return this == Root();
}

SceneObject* SceneObject::CreateObject() {
    std::shared_ptr<SceneObject> obj(new SceneObject(this));
    obj->SetController(controller);
    objects.push_back(obj);
    return obj.get();
}

void SceneObject::Erase(SceneObject* child) {
    for(unsigned i = 0; i < objects.size(); ++i)
    {
        if(objects[i].get() == child)
        {
            objects.erase(objects.begin() + i);
            break;
        }
    }
}

SceneObject::Component* SceneObject::Get(const std::string& component) {
    rttr::type type = rttr::type::get_by_name(component);
    if(!type.is_valid())
    {
        std::cout << component << " is not a valid type" << std::endl;
        return 0;
    }
    SceneObject::Component* c = FindComponent(type);
    if(!c)
    {
        rttr::variant v = type.create();
        if(!v.get_type().is_pointer())
        {
            std::cout << component << " - invalid component type" << std::endl;
            return 0;
        }
        c = v.get_value<SceneObject::Component*>();
        if(!c) return 0;
        c->type = type;
        AddComponent(c, type);
        return c;
    }
    else
        return c;
}

SceneObject::Component* SceneObject::FindComponent(rttr::type t) {
    std::map<rttr::type, SceneObject::Component*>::iterator it;
    it = components.find(t);
    if(it == components.end())
        return 0;
    else
        return it->second;
}

void SceneObject::Name(const std::string& name) { 
    this->name = name;
    if(this->name.empty()) this->name = MKSTR(this);
}

std::string SceneObject::Name() const { 
    return name; 
}

SceneObject* SceneObject::FindObject(const std::string& name) {
    SceneObject* o = 0;
    for(unsigned i = 0; i < objects.size(); ++i)
    {
        if(objects[i]->Name() == name)
        {
            o = objects[i].get();
            break;
        }
        else if(o = objects[i]->FindObject(name))
        {
            break;
        }
    }
    return o;
}

SceneObject* SceneObject::FindChild(const std::string& name) {
    for(auto so : objects)
    {
        if(so->Name() == name)
        {
            return so.get();
        }
    }
    return 0;
}

SceneObject* SceneObject::CreateFrom(SceneObject* from) {
    if(!from) return 0;
    SceneObject* new_object = CreateObject();
    new_object->SetController(controller);
    new_object->Name(from->Name());
    for(auto so : from->objects)
    {
        new_object->CreateFrom(so.get());
    }
    for(auto kv : from->components)
    {
        SceneObject::Component* c = kv.second->clone();
        if(!c) continue;
        components.insert(
            std::make_pair(
                kv.first, 
                c
            )
        );
    }
}

unsigned int SceneObject::ChildCount() const { 
    return objects.size(); 
}

SceneObject* SceneObject::GetChild(unsigned int i) const { 
    return objects[i].get(); 
}

unsigned int SceneObject::ComponentCount() const { 
    return components.size(); 
}

SceneObject::Component* SceneObject::GetComponent(unsigned int id) const {
    SceneObject::Component* c = 0;
    auto it = components.begin();
    for(unsigned i = 0; i < id; ++i)
    {
        it++;
    }
    return it->second;
}

void SceneObject::SetController(SceneController* con) {
    controller = con;
    for(auto& o : objects)
    {
        o->SetController(con);
    }
}

std::weak_ptr<SceneObject> SceneObject::WeakPtr() {
    return shared_from_this();
}

void SceneObject::AddComponent(SceneObject::Component* c, rttr::type t) {
    c->object = this;
    components.insert(std::make_pair(t, c));
    c->OnInit();
    if(controller)
    {
        controller->_onAddComponent(t, c, this);
    }
}