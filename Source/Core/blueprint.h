#ifndef BLUEPRINT_H
#define BLUEPRINT_H

#include <string>
#include <vector>
#include <map>
#include <component.h>

class BlueprintComponent
{
public:
    Meta meta;
};

class Blueprint
{
public:
    void Name(const std::string& name)
    {
        this->name = name;
    }
    std::string Name() const
    {
        return name;
    }
    void AddComponent(const std::string& name)
    {
        Meta meta = Meta::Get(name);
        BlueprintComponent bpc;
        bpc.meta = meta;
        components[meta.TypeIndex()] = bpc;
    }
    Blueprint& AddObject(const std::string& name)
    {
        objects.emplace_back(Blueprint());
        return objects.back();
    }
    std::map<typeindex, BlueprintComponent>& GetComponents()
    {
        return components;
    }
    std::vector<Blueprint>& GetObjects()
    {
        return objects;
    }
private:
    std::string name;
    std::map<typeindex, BlueprintComponent> components;
    std::vector<Blueprint> objects;
};

#endif
