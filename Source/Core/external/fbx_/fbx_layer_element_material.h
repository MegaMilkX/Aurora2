#ifndef FBX_LAYER_ELEMENT_MATERIAL_2_H
#define FBX_LAYER_ELEMENT_MATERIAL_2_H

#include "fbx_object.h"
#include "fbx_log.h"
#include "fbx_macro.h"
#include "fbx_layer_element_common.h"

namespace Fbx {

class LayerElementMaterial : public BasicObject {
public:
    virtual bool Make(Node& node) {
        Node* mappingNode = node.FindNode("MappingInformationType");
        Node* refTypeNode = node.FindNode("ReferenceInformationType");
        Node* materialsNode = node.FindNode("Materials");
        // TODO: MATERIALS INDEX, DOES IT EVEN EXIST?
        Node* materialsIndexNode = node.FindNode("MaterialsIndex");
        if(mappingNode) {
            mapping = LayerMappingFromString(mappingNode->GetProperty(0).GetString());
        }
        if(refTypeNode) {
            refType = RefTypeFromString(refTypeNode->GetProperty(0).GetString());
        }
        if(materialsNode) {
            materials = materialsNode->GetProperty(0).GetArray<int32_t>();
        }
        if(materialsIndexNode) {
            // TODO: Research
            //materialsIndex = materialsIndexNode->GetProperty(0).GetArray<int32_t>();
        }
        return true;
    }

    static bool IdentifyNode(Node& node) {
        if(node.GetName() != "LayerElementMaterial"
            || node.PropCount() < 1
            || !node.GetProperty(0).IsInt32()) 
            return false;
        return true;
    }

    LayerMapping mapping;
    LayerRefType refType;
    std::vector<int32_t> materials;
};

}

#endif