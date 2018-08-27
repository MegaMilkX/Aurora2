#ifndef FBX_GEOMETRY_H
#define FBX_GEOMETRY_H

#include "fbx_object.h"
#include "fbx_node.h"
#include <string>
#include <vector>
#include <iostream>

inline bool FbxTriangulate(std::vector<uint32_t>& out, std::vector<int32_t>& polygons)
{
    std::vector<int32_t> poly;
    for(auto fbxi : polygons)
    {
        int32_t i = fbxi < 0 ? -fbxi - 1 : fbxi;
        poly.emplace_back(i);

        if(fbxi < 0) {
            if(poly.size() < 3) {
                poly.clear();
                continue;
            }

            for(size_t p = 0; p < poly.size() - 2; ++p)
            {
                out.emplace_back(poly[0]);
                out.emplace_back(poly[p + 1]);
                out.emplace_back(poly[p + 2]);
            }

            poly.clear();
        }
    }

    return true;
}

struct FbxVertexMapping
{
    int32_t vertex;
    std::vector<int32_t> normals;
    std::vector<int32_t> uvs;

    bool operator==(const FbxVertexMapping& other) {
        if(vertex != other.vertex)
            return false;
        if(normals.size() != other.normals.size())
            return false;
        for(unsigned i = 0; i < normals.size(); ++i) {
            if(normals[i] != other.normals[i])
                return false;
        }
        if(uvs.size() != other.uvs.size())
            return false;
        for(unsigned i = 0; i < uvs.size(); ++i) {
            if(uvs[i] != other.uvs[i])
                return false;
        }
        return true;
    }
};

class FbxGeometry : public FbxObject
{
public:
    void SetName(const std::string& name) { this->name = name; }
    const std::string& GetName() const { return name; }

    size_t VertexCount() const { return vertexCount; }
    size_t IndexCount() const { return indexCount; }

    const std::vector<float>& GetVertices() const { return vertices; }
    const std::vector<uint32_t>& GetIndices() const { return indices; }

    size_t NormalLayerCount() const { return normal_layers.size(); }
    size_t UVLayerCount() const { return uv_layers.size(); }
    size_t RGBLayerCount() const { return rgb_layers.size(); }

    const std::vector<float>& GetNormals(int layer = 0) const { return normal_layers[layer]; }
    const std::vector<float>& GetUV(int layer = 0) const { return uv_layers[layer]; }
    const std::vector<float>& GetRGB(int layer = 0) const { return rgb_layers[layer]; }

    virtual bool Make(FbxNode& node);
private:
    std::string name;
    std::vector<float> vertices;
    std::vector<uint32_t> indices;
    std::vector<std::vector<float>> normal_layers;
    std::vector<std::vector<float>> uv_layers;
    std::vector<std::vector<float>> rgb_layers;
    size_t vertexCount;
    size_t indexCount;
};

#endif
