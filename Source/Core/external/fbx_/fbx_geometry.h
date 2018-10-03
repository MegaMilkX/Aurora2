#ifndef FBX_GEOMETRY_2_H
#define FBX_GEOMETRY_2_H

#include "fbx_object.h"
#include "fbx_log.h"
#include "fbx_macro.h"

#include "fbx_layer_element_normal.h"
#include "fbx_layer_element_color.h"
#include "fbx_layer_element_uv.h"
#include "fbx_layer_element_material.h"

#include "fbx_indexed_triangle_mesh.h"

namespace Fbx {

inline bool Triangulate(std::vector<Polygon>& out, const std::vector<Polygon>& polys) {
    for(auto& poly : polys) {
        for(int i = 0; i < poly.vertices.size() - 2; ++i) {
            Polygon new_poly;
            new_poly.Add(poly.vertices[0]);
            new_poly.Add(poly.vertices[i + 1]);
            new_poly.Add(poly.vertices[i + 2]);
            out.emplace_back(new_poly);
        }
    }
    
    return true;
}

class Geometry : public Object {
public:
    virtual bool Make(Node& node) {
        const std::string indexNodeName = "PolygonVertexIndex";
        const std::string vertexNodeName = "Vertices";

        std::vector<int32_t> fbxIndices;
        Node* indexNode = node.FindNode(indexNodeName);
        if(indexNode) {
            if(indexNode->PropCount()) {
                NodeProperty& p = indexNode->GetProperty(0);
                if(p.IsInt32Array()) {
                    fbxIndices = p.GetArray<int32_t>();
                } else {
                    FBX_LOGW("Unexpected geometry index format");
                    return false;
                }
            } else {
                FBX_LOGW("Geometry's indices node has no properties");
                return false;
            }
        } else {
            FBX_LOGW("Geometry has no index node");
            return false;
        }
        std::vector<double> fbxVertices;
        Node* vertexNode = node.FindNode(vertexNodeName);
        if(vertexNode) {
            if(vertexNode->PropCount()) {
                NodeProperty& p = vertexNode->GetProperty(0);
                if(p.IsDoubleArray()) {
                    fbxVertices = p.GetArray<double>();
                } else {
                    FBX_LOGW("Unexpected geometry vertex format");
                    return false;
                }
            } else {
                FBX_LOGW("Geometry's vertex node has no properties");
                return false;
            }
        } else {
            FBX_LOGW("Geometry has no vertex node");
            return false;
        }
        if(fbxVertices.size() % 3) {
            FBX_LOGW("Geometry's vertex array property size is not multiple of 3");
            return false;
        }
        for(size_t i = 0; i < fbxVertices.size(); i+=3) {
            controlPoints.emplace_back(
                FbxVector3(
                    (float)fbxVertices[i], 
                    (float)fbxVertices[i+1], 
                    (float)fbxVertices[i+2]
                )// * scene->Settings().scaleFactor
            );
        }

        size_t normalLayerCount = node.CountConvertable<LayerElementNormal>();
        size_t uvLayerCount = node.CountConvertable<LayerElementUV>();
        size_t colorLayerCount = node.CountConvertable<LayerElementColor>();
        size_t materialLayerCount = node.CountConvertable<LayerElementMaterial>();
        for(size_t i = 0; i < normalLayerCount; ++i) {
            LayerElementNormal* l = node.ConvertChild<LayerElementNormal>(i);
            normalLayers.emplace_back(l);
        }
        for(size_t i = 0; i < uvLayerCount; ++i) {
            LayerElementUV* l = node.ConvertChild<LayerElementUV>(i);
            uvLayers.emplace_back(l);
        }
        for(size_t i = 0; i < colorLayerCount; ++i) {
            LayerElementColor* l = node.ConvertChild<LayerElementColor>(i);
            rgbLayers.emplace_back(l);
        }
        for(size_t i = 0; i < materialLayerCount; ++i) {
            LayerElementMaterial* l = node.ConvertChild<LayerElementMaterial>(i);
            materialLayers.emplace_back(l);
        }

        Polygon poly;
        for(size_t i = 0; i < fbxIndices.size(); ++i) {
            int32_t index = fbxIndices[i];
            int32_t index_abs = index < 0 ? -index - 1 : index;
            unindexedVertices.emplace_back(
                Vertex{
                    index_abs, 
                    {
                        std::vector<int32_t>(normalLayers.size()), 
                        std::vector<int32_t>(uvLayers.size()), 
                        std::vector<int32_t>(rgbLayers.size()),
                        std::vector<int32_t>(materialLayers.size())
                    }
                }
            );
            poly.Add(unindexedVertices.size() - 1);
            if(index < 0) {
                if(poly.VertexCount() < 3) continue;
                polygons.emplace_back(poly);
                poly.Clear();
                continue;
            }
        }
        std::cout << "FBX: " << polygons.size() << " polygons built" << std::endl;

        for(size_t i = 0; i < normalLayers.size(); ++i) {
            normalLayers[i]->Fill(i, unindexedVertices, polygons);
        }
        for(size_t i = 0; i < uvLayers.size(); ++i) {
            uvLayers[i]->Fill(i, unindexedVertices, polygons);
        }
        for(size_t i = 0; i < rgbLayers.size(); ++i) {
            rgbLayers[i]->Fill(i, unindexedVertices, polygons);
        }
        
        return true;
    }

    static bool IdentifyNode(Node& node) {
        if(node.GetName() != "Geometry"
            || node.PropCount() < 3
            || !node.GetProperty(0).IsInt64()
            || !node.GetProperty(1).IsString()
            || !node.GetProperty(2).IsString()
            || node.GetProperty(2).GetString != "Mesh") 
            return false;
        return true;
    }

    IndexedTriangleMesh MakeIndexedMesh() {
        IndexedTriangleMesh mesh;

        mesh.normal_layers.resize(1);
        mesh.uv_layers.resize(1);
        
        std::vector<Polygon> polys;
        Triangulate(polys, polygons);

        //FbxSkin* skin = GetSkin();

        int32_t index = 0;
        for(size_t i = 0; i < polys.size(); ++i) {
            for(size_t j = 0; j < polys[i].vertices.size(); ++j) {
                Vertex& vert = unindexedVertices[polys[i].vertices[j]];
                mesh.vertices.emplace_back(controlPoints[vert.controlPoint].x);
                mesh.vertices.emplace_back(controlPoints[vert.controlPoint].y);
                mesh.vertices.emplace_back(controlPoints[vert.controlPoint].z);

                if(normalLayers.size() > 0) {
                    mesh.normal_layers[0].emplace_back((float)normalLayers[0]->normals[vert.attribs[Vertex::Normal][0]*3]);
                    mesh.normal_layers[0].emplace_back((float)normalLayers[0]->normals[vert.attribs[Vertex::Normal][0]*3+1]);
                    mesh.normal_layers[0].emplace_back((float)normalLayers[0]->normals[vert.attribs[Vertex::Normal][0]*3+2]);
                }
                if(uvLayers.size() > 0) {
                    mesh.uv_layers[0].emplace_back((float)uvLayers[0]->uv[vert.attribs[Vertex::UV][0]*2]);
                    mesh.uv_layers[0].emplace_back((float)uvLayers[0]->uv[vert.attribs[Vertex::UV][0]*2+1]);
                }

                /*
                if(skin) {
                    std::vector<int32_t> bi(4);
                    std::vector<float> bw(4);
                    if(skin->boneDataPerControlPoint.size() <= vert.controlPoint) {
                        std::cout << "ERROR: Bone data per control point array is too small" << std::endl;
                    }
                    for(size_t k = 0; k < skin->boneDataPerControlPoint[vert.controlPoint].size() && k < 4; ++k) {
                        bi[k] = skin->boneDataPerControlPoint[vert.controlPoint][k].bone;
                        bw[k] = skin->boneDataPerControlPoint[vert.controlPoint][k].weight;
                    }
                    mesh.boneIndices4.insert(mesh.boneIndices4.end(), bi.begin(), bi.end());
                    mesh.boneWeights4.insert(mesh.boneWeights4.end(), bw.begin(), bw.end());
                }
                */
                mesh.indices.emplace_back(index);
                index++;
            }
        }

        return mesh;
    }
private:
    std::vector<Vertex> unindexedVertices;

    std::vector<FbxVector3> controlPoints;
    std::vector<Polygon> polygons;
    std::vector<LayerElementNormal*> normalLayers;
    std::vector<LayerElementUV*> uvLayers;
    std::vector<LayerElementColor*> rgbLayers;
    std::vector<LayerElementMaterial*> materialLayers;
};

}

#endif