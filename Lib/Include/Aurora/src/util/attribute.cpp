#include "attribute.h"

#include <map>

namespace Au{

std::map<std::pair<typeindex, AttribInstance>, AttribIndex> globalAttribIndexMap;
AttribIndex GetGlobalAttribIndex(typeindex type, unsigned int instance)
{
    std::pair<typeindex, AttribInstance> key = std::make_pair(type, instance);
    std::map<std::pair<typeindex, AttribInstance>, AttribIndex>::iterator it;
    it = globalAttribIndexMap.find(key);
    if(it == globalAttribIndexMap.end())
    {
        AttribIndex new_index = globalAttribIndexMap.size();
        globalAttribIndexMap[key] = new_index;
        return new_index;
    }
    else
    {
        return it->second;
    }
}

}
