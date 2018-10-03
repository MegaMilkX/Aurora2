#ifndef FBX_LAYER_ELEMENT_COLOR_2_H
#define FBX_LAYER_ELEMENT_COLOR_2_H

#include "fbx_object.h"
#include "fbx_log.h"
#include "fbx_macro.h"
#include "fbx_layer_element_common.h"

namespace Fbx {

class LayerElementColor : public BasicObject {
public:
    virtual bool Make(Node& node) {
        Node* mappingNode = node.FindNode("MappingInformationType");
        Node* refTypeNode = node.FindNode("ReferenceInformationType");
        Node* colorsNode = node.FindNode("Colors");
        Node* colorIndexNode = node.FindNode("ColorIndex");
        if(mappingNode) {
            mapping = LayerMappingFromString(mappingNode->GetProperty(0).GetString());
        }
        if(refTypeNode) {
            refType = RefTypeFromString(refTypeNode->GetProperty(0).GetString());
        }
        if(colorsNode) {
            colors = colorsNode->GetProperty(0).GetArray<double>();
        }
        if(colorIndexNode) {
            colorIndex = colorIndexNode->GetProperty(0).GetArray<int32_t>();
        }
        return true;
    }

    static bool IdentifyNode(Node& node) {
        if(node.GetName() != "LayerElementColor"
            || node.PropCount() < 1
            || !node.GetProperty(0).IsInt32()) 
            return false;
        return true;
    }

    void Fill(
        int i, 
        std::vector<Vertex>& unindexedVertices, 
        std::vector<Polygon>& polygons
    ) {
        int elemSize = 3;
        Vertex::Attrib attr = Vertex::RGB;
        if(mapping == ByVertex) {
            // Element for each vertex
            // TODO: WRONG, it's for each CONTROL POINT
            if(refType == Direct) {
                for(unsigned j = 0; j < colors.size() / elemSize; ++j) {
                    for(auto& v : unindexedVertices) {
                        if(v.controlPoint == j) {
                            v.attribs[attr][i] = j;
                        }
                    }
                }
            } else if(refType == IndexToDirect) {
                for(unsigned j = 0; j < colorIndex.size(); ++j) {
                    for(auto& v : unindexedVertices) {
                        if(v.controlPoint == j) {
                            v.attribs[attr][i] = colorIndex[j];
                        }
                    }
                }
            }
        } else if(mapping == ByPolygon) {
            // Element for each polygon
            size_t elem_count = colors.size() / elemSize;
            if(polygons.size() != elem_count) {
                std::cout << "FBX | NormalLayer " <<  i << ": polycount mismatch with ByPolygon mapping" << std::endl;
                return;
            }
            for(size_t j = 0; j < elem_count; ++j) {
                for(size_t k = 0; k < polygons[j].vertices.size(); ++k) {
                    Vertex& vertex = unindexedVertices[polygons[j].vertices[k]];
                    vertex.attribs[attr][i] = j;
                }
            }
        } else if(mapping == ByPolygonVertex) {
            // Element for each vertex in a polygon
            if(refType == Direct) {
                for(unsigned j = 0; j < colors.size() / elemSize; ++j) {
                    unindexedVertices[j].attribs[attr][i] = j;
                }
            } else if(refType == IndexToDirect) {
                for(unsigned j = 0; j < colorIndex.size(); ++j) {
                    unindexedVertices[j].attribs[attr][i] = colorIndex[j];
                }
            }
        }
    }

    LayerMapping mapping;
    LayerRefType refType;
    std::vector<double> colors;
    std::vector<int32_t> colorIndex;
};

}

#endif