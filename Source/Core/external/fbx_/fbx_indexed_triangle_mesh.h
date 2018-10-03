#ifndef FBX_INDEXED_TRIANGLE_MESH_2_H
#define FBX_INDEXED_TRIANGLE_MESH_2_H

#include <vector>
#include <stdint.h>

namespace Fbx {

class Geometry;
class IndexedTriangleMesh {
    friend Geometry;
public:
    size_t VertexCount() const { return vertices.size() / 3; }
    size_t IndexCount() const { return indices.size(); }

    const std::vector<float>& GetVertices() const { return vertices; }
    const std::vector<uint32_t>& GetIndices() const { return indices; }

    size_t NormalLayerCount() const { return normal_layers.size(); }
    size_t UVLayerCount() const { return uv_layers.size(); }
    size_t RGBLayerCount() const { return rgb_layers.size(); }

    const std::vector<float>& GetNormals(int layer = 0) const { return normal_layers[layer]; }
    const std::vector<float>& GetUV(int layer = 0) const { return uv_layers[layer]; }
    const std::vector<float>& GetRGB(int layer = 0) const { return rgb_layers[layer]; }

    const std::vector<int32_t>& GetBoneIndices4() const { return boneIndices4; }
    const std::vector<float>& GetBoneWeights4() const { return boneWeights4; }
private:
    std::vector<float> vertices;
    std::vector<uint32_t> indices;
    std::vector<std::vector<float>> normal_layers;
    std::vector<std::vector<float>> uv_layers;
    std::vector<std::vector<float>> rgb_layers;
    
    std::vector<int32_t> boneIndices4;
    std::vector<float> boneWeights4;
};

}

#endif
