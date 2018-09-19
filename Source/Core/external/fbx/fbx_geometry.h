#ifndef FBX_GEOMETRY_H
#define FBX_GEOMETRY_H

#include "fbx_object.h"
#include "fbx_node.h"
#include "fbx_math.h"
#include <string>
#include <vector>
#include <iostream>
#include "fbx_indexed_triangle_mesh.h"
#include "fbx_skin.h"

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

struct FbxVertex {
    enum Attrib {
        Normal,
        UV,
        RGB,
        Material
    };
    
    int32_t controlPoint;
    std::vector<int32_t> attribs[4];
};

class FbxPolygon {
public:
    void Add(size_t v) { vertices.emplace_back(v); }
    size_t VertexCount() const { return vertices.size(); }
    size_t VertexIndex(size_t i) { return vertices[i]; }
    void Clear() { vertices.clear(); }

    std::vector<size_t> vertices;
    int64_t materialUid;
};

inline bool FbxTriangulate(std::vector<FbxPolygon>& out, const std::vector<FbxPolygon>& polys) {
    for(auto& poly : polys) {
        for(int i = 0; i < poly.vertices.size() - 2; ++i) {
            FbxPolygon new_poly;
            new_poly.Add(poly.vertices[0]);
            new_poly.Add(poly.vertices[i + 1]);
            new_poly.Add(poly.vertices[i + 2]);
            out.emplace_back(new_poly);
        }
    }
    
    return true;
}

enum FbxLayerMapping {
    FbxMappingUnknown,
    FbxByVertex,
    FbxByPolygon,
    FbxByPolygonVertex
};
enum FbxLayerRefType {
    FbxRefTypeUnknown,
    FbxIndexToDirect,
    FbxDirect
};

template<typename T>
class FbxGeometryLayerElement {
public:
    FbxLayerMapping mapping;
    FbxLayerRefType refType;
    std::vector<T> elements;
    std::vector<int32_t> indices;

    bool Make(FbxNode& node, const std::string& elemName, const std::string indicesName) {
        mapping = MappingFromString(
            node.GetNode("MappingInformationType", 0)
                .GetProperty(0)
                .GetString()
        );
        refType = RefTypeFromString(
            node.GetNode("ReferenceInformationType", 0)
                .GetProperty(0)
                .GetString()
        );
        if(node.ChildCount(elemName) == 0) return false;
        elements = node.GetNode(elemName, 0)
            .GetProperty(0)
            .GetArray<T>();
        if(node.ChildCount(indicesName)) {
            FbxNode& fbxIndices = node.GetNode(indicesName, 0);
            indices = fbxIndices.GetProperty(0).GetArray<int32_t>();
        }
        return true;
    }

    static FbxLayerMapping MappingFromString(const std::string& s) {
        if(s == "ByVertex" || s == "ByVertice")
            return FbxByVertex;
        else if(s == "ByPolygon")
            return FbxByPolygon;
        else if(s == "ByPolygonVertex")
            return FbxByPolygonVertex;
        return FbxMappingUnknown;
    }

    static FbxLayerRefType RefTypeFromString(const std::string& s) {
        if(s == "IndexToDirect" || s == "Index")
            return FbxIndexToDirect;
        else if(s == "Direct")
            return FbxDirect;
        return FbxRefTypeUnknown;
    }
};

class FbxGeometry : public FbxObject
{
public:
    virtual const char* Type() const { return "Geometry"; }
    virtual bool Make(FbxNode& node);

    void SetName(const std::string& name) { this->name = name; }
    const std::string& GetName() const { return name; }

    FbxIndexedTriangleMesh MakeIndexedMesh();
    bool HasSkin() { return skin != 0; }
    FbxSkin* GetSkin();
private:
    std::string name;

    std::vector<FbxVertex> unindexedVertices;

    std::vector<FbxVector3> controlPoints;
    std::vector<FbxPolygon> polygons;
    std::vector<FbxGeometryLayerElement<double>> normalLayers;
    std::vector<FbxGeometryLayerElement<double>> uvLayers;
    std::vector<FbxGeometryLayerElement<double>> rgbLayers;
    std::vector<FbxGeometryLayerElement<int32_t>> materialLayers;

    int64_t skin = 0;
};

#endif
