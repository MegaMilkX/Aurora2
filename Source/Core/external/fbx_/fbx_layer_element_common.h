#ifndef FBX_LAYER_ELEMENT_COMMON_H
#define FBX_LAYER_ELEMENT_COMMON_H

namespace Fbx {

struct Vertex {
    enum Attrib {
        Normal,
        UV,
        RGB,
        Material
    };
    
    int32_t controlPoint;
    std::vector<int32_t> attribs[4];
};

class Polygon {
public:
    void Add(size_t v) { vertices.emplace_back(v); }
    size_t VertexCount() const { return vertices.size(); }
    size_t VertexIndex(size_t i) { return vertices[i]; }
    void Clear() { vertices.clear(); }

    std::vector<size_t> vertices;
    int64_t materialUid;
};

enum LayerMapping {
    LayerMappingUnknown,
    ByVertex,
    ByPolygon,
    ByPolygonVertex
};
enum LayerRefType {
    RefTypeUnknown,
    IndexToDirect,
    Direct
};

inline LayerMapping LayerMappingFromString(const std::string& s) {
    if(s == "ByVertex" || s == "ByVertice")
        return ByVertex;
    else if(s == "ByPolygon")
        return ByPolygon;
    else if(s == "ByPolygonVertex")
        return ByPolygonVertex;
    return LayerMappingUnknown;
}

inline LayerRefType RefTypeFromString(const std::string& s) {
    if(s == "IndexToDirect" || s == "Index")
        return IndexToDirect;
    else if(s == "Direct")
        return Direct;
    return RefTypeUnknown;
}

}

#endif
