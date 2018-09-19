#include "fbx_geometry.h"
#include "fbx_scene.h"

template<typename T>
inline void FillLayer(
    FbxGeometryLayerElement<T>& l, 
    FbxVertex::Attrib attr, 
    int i, 
    std::vector<FbxVertex>& unindexedVertices, 
    std::vector<FbxPolygon>& polygons,
    int elemSize
) {
    if(l.mapping == FbxByVertex) {
        // Element for each vertex
        // TODO: WRONG, it's for each CONTROL POINT
        if(l.refType == FbxDirect) {
            for(unsigned j = 0; j < l.elements.size() / elemSize; ++j) {
                //unindexedVertices[j].normalLayers[i] = j;
                for(auto& v : unindexedVertices) {
                    if(v.controlPoint == j) {
                        v.attribs[attr][i] = j;
                    }
                }
            }
        } else if(l.refType == FbxIndexToDirect) {
            for(unsigned j = 0; j < l.indices.size(); ++j) {
                //unindexedVertices[j].normalLayers[i] = l.indices[j];
                for(auto& v : unindexedVertices) {
                    if(v.controlPoint == j) {
                        v.attribs[attr][i] = l.indices[j];
                    }
                }
            }
        }
    } else if(l.mapping == FbxByPolygon) {
        // Element for each polygon
        size_t elem_count = l.elements.size() / elemSize;
        if(polygons.size() != elem_count) {
            std::cout << "FBX | NormalLayer " <<  i << ": polycount mismatch with ByPolygon mapping" << std::endl;
            return;
        }
        for(size_t j = 0; j < elem_count; ++j) {
            for(size_t k = 0; k < polygons[j].vertices.size(); ++k) {
                FbxVertex& vertex = unindexedVertices[polygons[j].vertices[k]];
                vertex.attribs[attr][i] = j;
            }
        }
    } else if(l.mapping == FbxByPolygonVertex) {
        // Element for each vertex in a polygon
        if(l.refType == FbxDirect) {
            for(unsigned j = 0; j < l.elements.size() / elemSize; ++j) {
                unindexedVertices[j].attribs[attr][i] = j;
            }
        } else if(l.refType == FbxIndexToDirect) {
            for(unsigned j = 0; j < l.indices.size(); ++j) {
                unindexedVertices[j].attribs[attr][i] = l.indices[j];
            }
        }
    }
}

bool FbxGeometry::Make(FbxNode& node) {
    if(!scene) return false;
    auto scaleFactor = scene->Settings().scaleFactor;
    int64_t uid = node.GetProperty(0).GetInt64();
    std::string name = node.GetProperty(1).GetString();
    SetUid(uid);
    SetName(name);
    const std::string indexNodeName = "PolygonVertexIndex";
    const std::string vertexNodeName = "Vertices";
    const std::string normalLayerNodeName = "LayerElementNormal";
    const std::string uvLayerNodeName = "LayerElementUV";
    const std::string materialLayerNodeName = "LayerElementMaterial";

    if(!node.ChildCount(indexNodeName))
        return false;
    std::vector<int32_t> fbxIndices =
        node.GetNode(indexNodeName, 0)
            .GetProperty(0)
            .GetArray<int32_t>();

    std::vector<double> fbxVertices;
    if(!node.ChildCount(vertexNodeName))
        return false;
    fbxVertices =
        node.GetNode(vertexNodeName, 0)
            .GetProperty(0)
            .GetArray<double>();
    if(fbxVertices.size() % 3) {
        std::cout << "FBX: fbxVertices.size() % 3 is not null!" << std::endl;
        return false;
    }
    for(size_t i = 0; i < fbxVertices.size(); i+=3) {
        controlPoints.emplace_back(
            FbxVector3(fbxVertices[i], fbxVertices[i+1], fbxVertices[i+2]) * scene->Settings().scaleFactor
        );
    }
    std::cout << "FBX: " << controlPoints.size() << " control points loaded" << std::endl;

    normalLayers.resize(node.ChildCount(normalLayerNodeName));
    uvLayers.resize(node.ChildCount(uvLayerNodeName));
    materialLayers.resize(node.ChildCount(materialLayerNodeName));
    std::cout << "FBX: normal layer count: " << normalLayers.size() << std::endl;
    std::cout << "FBX: uv layer count: " << uvLayers.size() << std::endl;
    std::cout << "FBX: material layer count: " << materialLayers.size() << std::endl;

    for(unsigned i = 0; i < node.ChildCount(normalLayerNodeName); ++i) {
        FbxNode& fbxLayer = node.GetNode(normalLayerNodeName, i);
        normalLayers[i].Make(fbxLayer, "Normals", "NormalsIndex");
    }
    for(unsigned i = 0; i < node.ChildCount(uvLayerNodeName); ++i) {
        FbxNode& fbxLayer = node.GetNode(uvLayerNodeName, i);
        uvLayers[i].Make(fbxLayer, "UV", "UVIndex");
    }
    for(unsigned i = 0; i < node.ChildCount(materialLayerNodeName); ++i) {
        FbxNode& fbxLayer = node.GetNode(materialLayerNodeName, i);
        materialLayers[i].Make(fbxLayer, "Materials", "MaterialsIndex");
    }

    // Build polygons and unindexed vertices
    FbxPolygon poly;
    for(size_t i = 0; i < fbxIndices.size(); ++i) {
        int32_t index = fbxIndices[i];
        int32_t index_abs = index < 0 ? -index - 1 : index;
        unindexedVertices.emplace_back(
            FbxVertex{
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
    // =====

    for(size_t i = 0; i < normalLayers.size(); ++i) {
        auto& l = normalLayers[i];
        FillLayer(l, FbxVertex::Normal, i, unindexedVertices, polygons, 3);
    }
    for(size_t i = 0; i < uvLayers.size(); ++i) {
        auto& l = uvLayers[i];
        FillLayer(l, FbxVertex::UV, i, unindexedVertices, polygons, 2);
    }
    

/*
    std::vector<int32_t> fbxIndices;
    if(!node.ChildCount(indexNodeName))
        return false;
    fbxIndices =
        node.GetNode(indexNodeName, 0)
            .GetProperty(0)
            .GetArray<int32_t>();
    // TODO: Triangulate later
    FbxTriangulate(indices, fbxIndices);

    std::vector<double> fbxVertices;
    if(!node.ChildCount(vertexNodeName))
        return false;
    fbxVertices =
        node.GetNode(vertexNodeName, 0)
            .GetProperty(0)
            .GetArray<double>();
    vertices.reserve(fbxVertices.size());
    for(auto d : fbxVertices) vertices.emplace_back((float)d * (float)scaleFactor);

    normal_layers.resize(node.ChildCount(normalLayerNodeName));
    for(auto& l : normal_layers) l.resize(vertices.size());
    uv_layers.resize(node.ChildCount(uvLayerNodeName));
    for(auto& l : uv_layers) l.resize(vertices.size() / 3 * 2);

    if(normal_layers.size() == 0)
    {
        normal_layers.resize(1);
        for(auto& l : normal_layers) l.resize(vertices.size());
    }
    if(uv_layers.size() == 0)
    {
        uv_layers.resize(1);
        for(auto& l : uv_layers) l.resize(vertices.size() / 3 * 2);
    }
    vertexCount = vertices.size() / 3;
    indexCount = indices.size();

    for(unsigned i = 0; i < node.ChildCount(normalLayerNodeName); ++i) {
        FbxNode& fbxLayer = node.GetNode(normalLayerNodeName, i);
        std::vector<float>& normals = normal_layers[i];
        std::string mapping = 
            fbxLayer.GetNode("MappingInformationType", 0)
                .GetProperty(0)
                .GetString();
        std::string refType = 
            fbxLayer.GetNode("ReferenceInformationType", 0)
                .GetProperty(0)
                .GetString();
        std::vector<double> fbxNormals =
            fbxLayer.GetNode("Normals", 0)
                .GetProperty(0)
                .GetArray<double>();
        if(mapping == "ByVertex" || mapping == "ByVertice") {
            if(refType == "Direct") {
                for(unsigned l = 0; l < fbxNormals.size() / 3; ++l) {
                    normals[l * 3] = (float)(fbxNormals[l * 3]);
                    normals[l * 3 + 1] = (float)(fbxNormals[l * 3 + 1]);
                    normals[l * 3 + 2] = (float)(fbxNormals[l * 3 + 2]);
                }
            }
            else if(refType == "IndexToDirect" || refType == "Index") {
                FbxNode& fbxNormalsIndex = fbxLayer.GetNode("NormalsIndex", 0);
                std::vector<int32_t> normalIndices = 
                    fbxNormalsIndex.GetProperty(0).GetArray<int32_t>();
                for(unsigned l = 0; l < normalIndices.size(); ++l) {
                    normals[l * 3] = (float)fbxNormals[normalIndices[l] * 3];
                    normals[l * 3 + 1] = (float)fbxNormals[normalIndices[l] * 3 + 1];
                    normals[l * 3 + 2] = (float)fbxNormals[normalIndices[l] * 3 + 2];
                }
            }
        }
        else if(mapping == "ByPolygon") {

        }
        else if(mapping == "ByPolygonVertex") {
            if(refType == "Direct") {
                for(unsigned l = 0; l < fbxIndices.size(); ++l) {
                    int32_t index = fbxIndices[l];
                    index = index < 0 ? -index - 1 : index;
                    normals[index * 3] = (float)fbxNormals[l * 3];
                    normals[index * 3 + 1] = (float)fbxNormals[l * 3 + 1];
                    normals[index * 3 + 2] = (float)fbxNormals[l * 3 + 2];
                }
            }
            else if(refType == "IndexToDirect" || refType == "Index") {
                FbxNode& fbxNormalsIndex = fbxLayer.GetNode("NormalsIndex", 0);
                std::vector<int32_t> normalIndices = 
                    fbxNormalsIndex.GetProperty(0).GetArray<int32_t>();
                for(unsigned l = 0; l < normalIndices.size(); ++l) {
                    int32_t vindex = fbxIndices[l];
                    vindex = vindex < 0 ? -vindex - 1 : vindex;
                    int32_t nindex = normalIndices[l];
                    normals[vindex * 3] = (float)fbxNormals[nindex * 3];
                    normals[vindex * 3 + 1] = (float)fbxNormals[nindex * 3 + 1];
                    normals[vindex * 3 + 2] = (float)fbxNormals[nindex * 3 + 2];
                }
            }
        }
    }

    for(unsigned i = 0; i < node.ChildCount(uvLayerNodeName); ++i) {
        FbxNode& fbxLayer = node.GetNode(uvLayerNodeName, i);
        std::vector<float>& uv = uv_layers[i];
        std::string mapping = 
            fbxLayer.GetNode("MappingInformationType", 0)
                .GetProperty(0)
                .GetString();
        std::string refType =
            fbxLayer.GetNode("ReferenceInformationType", 0)
                .GetProperty(0)
                .GetString();
        std::vector<double> fbxUV = 
            fbxLayer.GetNode("UV", 0)
                .GetProperty(0)
                .GetArray<double>();
        if(mapping == "ByVertex" || mapping == "ByVertice")
        {
            if(refType == "Direct") {
                for(unsigned l = 0; l < fbxUV.size() / 3; ++l) {
                    uv[l * 2] = (float)(fbxUV[l * 2]);
                    uv[l * 2 + 1] = (float)(fbxUV[l * 2 + 1]);
                }
            }
            else if(refType == "IndexToDirect" || refType == "Index") {
                FbxNode& fbxNormalsIndex = fbxLayer.GetNode("UVIndex", 0);
                std::vector<int32_t> uvIndices = 
                    fbxNormalsIndex.GetProperty(0).GetArray<int32_t>();
                for(unsigned l = 0; l < uvIndices.size(); ++l) {
                    uv[l * 2] = (float)fbxUV[uvIndices[l] * 2];
                    uv[l * 2 + 1] = (float)fbxUV[uvIndices[l] * 2 + 1];
                }
            }
        }
        else if(mapping == "ByPolygon")
        {

        }
        else if(mapping == "ByPolygonVertex")
        {
            if(refType == "Direct") {
                for(unsigned l = 0; l < fbxIndices.size(); ++l) {
                    int32_t index = fbxIndices[l];
                    index = index < 0 ? -index - 1 : index;
                    uv[index * 2] = (float)fbxUV[l * 2];
                    uv[index * 2 + 1] = (float)fbxUV[l * 2 + 1];
                }
            }
            else if(refType == "IndexToDirect" || refType == "Index") {
                FbxNode& fbxUVIndex = fbxLayer.GetNode("UVIndex", 0);
                std::vector<int32_t> uvIndices = 
                    fbxUVIndex.GetProperty(0).GetArray<int32_t>();
                for(unsigned l = 0; l < uvIndices.size(); ++l) {
                    int32_t vindex = fbxIndices[l];
                    vindex = vindex < 0 ? -vindex - 1 : vindex;
                    int32_t uindex = uvIndices[l];
                    uv[vindex * 2] = (float)fbxUV[uindex * 2];
                    uv[vindex * 2 + 1] = (float)fbxUV[uindex * 2 + 1];
                }
            }
        }
    }
*/
    return true;
}

FbxIndexedTriangleMesh FbxGeometry::MakeIndexedMesh() {
    FbxIndexedTriangleMesh mesh;

    mesh.normal_layers.resize(1);
    mesh.uv_layers.resize(1);
    
    std::vector<FbxPolygon> polys;
    FbxTriangulate(polys, polygons);

    int32_t index = 0;
    for(size_t i = 0; i < polys.size(); ++i) {
        for(size_t j = 0; j < polys[i].vertices.size(); ++j) {
            FbxVertex& vert = unindexedVertices[polys[i].vertices[j]];
            mesh.vertices.emplace_back(controlPoints[vert.controlPoint].x);
            mesh.vertices.emplace_back(controlPoints[vert.controlPoint].y);
            mesh.vertices.emplace_back(controlPoints[vert.controlPoint].z);

            if(normalLayers.size() > 0) {
                mesh.normal_layers[0].emplace_back((float)normalLayers[0].elements[vert.attribs[FbxVertex::Normal][0]*3]);
                mesh.normal_layers[0].emplace_back((float)normalLayers[0].elements[vert.attribs[FbxVertex::Normal][0]*3+1]);
                mesh.normal_layers[0].emplace_back((float)normalLayers[0].elements[vert.attribs[FbxVertex::Normal][0]*3+2]);
            }
            if(uvLayers.size() > 0) {
                mesh.uv_layers[0].emplace_back((float)uvLayers[0].elements[vert.attribs[FbxVertex::UV][0]*2]);
                mesh.uv_layers[0].emplace_back((float)uvLayers[0].elements[vert.attribs[FbxVertex::UV][0]*2+1]);
            }
            mesh.indices.emplace_back(index);
            index++;
        }
    }

    return mesh;
}