#include "component.h"

void InitComponentMeta()
{
    for(auto& kv : ComponentMetaMap())
    {
        std::cout << kv.first << std::endl;
    }
}

component_map_t& ComponentMetaMap(bool update, const std::string& name, int value)
{
    static component_map_t componentMap;
    if(update)
    {
        componentMap[name] = value;
    }
    return componentMap;
}