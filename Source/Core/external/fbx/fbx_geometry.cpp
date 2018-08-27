#include "fbx_geometry.h"
#include "fbx_scene.h"

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

    return true;
}