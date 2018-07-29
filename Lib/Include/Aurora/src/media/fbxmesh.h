#ifndef AU_FBXMESH_H
#define AU_FBXMESH_H

#include <vector>
#include <string>

#include "fbxsettings.h"

#include "fbxnode.h"
#include "fbxmodel.h"

#include "fbxskin.h"

#include "fbxpose.h"

namespace Au{
namespace Media{
namespace FBX{

class Mesh
{
public:
    Mesh() {}
    Mesh(Settings& settings, Node& rootNode, Node& geom)
    {
        uid = geom[0].GetInt64();
        
        Node* conn = 0;
        Node* model = 
            rootNode.GetConnectedParent("Model", uid, &conn);
        if(model)
        {
            name = (*model)[1].GetString();
        }
        
        Math::Mat4f bindTransform = Math::Mat4f(1.0f);
        Node* poseData = rootNode.GetWhere("Pose", 2, "BindPose");
        Pose pose(settings, &rootNode, poseData);
        
        if(!pose.GetPoseTransform((*model)[0].GetInt64(), bindTransform))
        {
            Model model(&settings, &rootNode, model);
            bindTransform = model.transform;
        }
        
        _getVerticesAndIndices(settings, rootNode, geom, bindTransform);
        _getNormals(settings, rootNode, geom, bindTransform);
        _getUV(rootNode, geom);
        
        //OptimizeDuplicates();
        
        skin = Skin(rootNode, geom);
    }
    
    int VertexCount() { return vertices.size() / 3; }
    
    std::vector<float> GetVertices()
    {
        std::vector<float> result = vertices;
        return result;
    }
    std::vector<float> GetNormals(unsigned layer)
    {
        if(layer >= normalLayers.size())
            return std::vector<float>();
        return normalLayers[layer];
    }
    std::vector<float> GetUV(unsigned layer)
    {
        if(layer >= uvLayers.size())
            return std::vector<float>();
        return uvLayers[layer];
    }
    template<typename T>
    std::vector<T> GetIndices()
    {
        std::vector<T> result;
        for(unsigned i = 0; i < indices.size(); ++i)
            result.push_back((T)indices[i]);
        return result;
    }
    
    Skin& GetSkin()
    {
        return skin;
    }
    
    void ConvertCoordSystem(Axis new_right, Axis new_up, Axis new_front)
    {
        ConvertVertexArray3(vertices, right, up, front,
                            new_right, new_up, new_front);
        for(unsigned i = 0; i < normalLayers.size(); ++i)
            ConvertVertexArray3(normalLayers[i], right, up, front,
                                new_right, new_up, new_front);
        
        this->right = new_right;
        this->up = new_up;
        this->front = new_front;
    }
    
    template<typename T>
    void ConvertVertexArray3(std::vector<T>& data,
                            Axis right, Axis up, Axis front,
                            Axis new_right, Axis new_up, Axis new_front)
    {
        for(unsigned i = 0; i < data.size(); i += 3)
        {
            int right_sign = right >= 0 && new_right >= 0 ||
                            right < 0 && new_right < 0 ?
                            1 : -1;
                            
            int up_sign = up >= 0 && new_up >= 0 ||
                            up < 0 && new_up < 0 ?
                            1 : -1;
                            
            int front_sign = front >= 0 && new_front >= 0 ||
                            front < 0 && new_front < 0 ?
                            1 : -1;
            
            T r = right >= 0 ? data[i + right] : data[i - (right + 1)];
            T u = up >= 0 ? data[i + up] : data[i - (up + 1)];
            T f = front >= 0 ? data[i + front] : data[i - (front + 1)];
            
            new_right = (Axis)(new_right < 0 ? -(new_right + 1) : new_right);
            new_up = (Axis)(new_up < 0 ? -(new_up + 1) : new_up);
            new_front = (Axis)(new_front < 0 ? -(new_front + 1) : new_front);
            
            data[i + new_right] = r * right_sign;
            data[i + new_up] = u * up_sign;
            data[i + new_front] = f * front_sign;
        }
    }

    void OptimizeDuplicates()
    {
        for(unsigned i = 0; i < indices.size(); ++i)
            for(unsigned j = i + 1; j < indices.size(); ++j)
                if(IsSameVertex(indices[i], indices[j]))
                    EraseVertex(indices[j], indices[i]);
    }
    
    bool IsSameVertex(unsigned a, unsigned b)
    {
        if(a == b)
            return false;
        
        if(vertices[a * 3] != vertices[b * 3] || 
           vertices[a * 3 + 1] != vertices[b * 3 + 1] ||
           vertices[a * 3 + 2] != vertices[b * 3 + 2])
            return false;
        else
        {
            for(unsigned i = 0; i < normalLayers.size(); ++i)
            {
                if(normalLayers[i][a * 3] != normalLayers[i][b * 3] || 
                   normalLayers[i][a * 3 + 1] != normalLayers[i][b * 3 + 1] ||
                   normalLayers[i][a * 3 + 2] != normalLayers[i][b * 3 + 2])
                    return false;
            }
            
            for(unsigned i = 0; i < uvLayers.size(); ++i)
            {
                if(uvLayers[i][a * 2] != uvLayers[i][b * 2] || 
                   uvLayers[i][a * 2 + 1] != uvLayers[i][b * 2 + 1])
                    return false;
            }
            
            for(unsigned i = 0; i < rgbaLayers.size(); ++i)
            {
                if(rgbaLayers[i][a * 4] != rgbaLayers[i][b * 4] || 
                   rgbaLayers[i][a * 4 + 1] != rgbaLayers[i][b * 4 + 1] ||
                   rgbaLayers[i][a * 4 + 2] != rgbaLayers[i][b * 4 + 2] ||
                   rgbaLayers[i][a * 4 + 3] != rgbaLayers[i][b * 4 + 3])
                    return false;
            }
        }
        
        return true;
    }
    
    void EraseVertex(int id, int newId)
    {
        for(unsigned i = 0; i < indices.size(); ++i)
            if(indices[i] == id)
                indices[i] = newId;
            
        for(unsigned i = 0; i < indices.size(); ++i)
            if(indices[i] >= id)
                indices[i]--;
            
        vertices.erase(vertices.begin() + id * 3, vertices.begin() + id * 3 + 3);
        origVertIndices.erase(origVertIndices.begin() + id);
        
        for(unsigned i = 0; i < normalLayers.size(); ++i)
            normalLayers[i].erase(normalLayers[i].begin() + id * 3, normalLayers[i].begin() + id * 3 + 3);
        
        for(unsigned i = 0; i < uvLayers.size(); ++i)
            uvLayers[i].erase(uvLayers[i].begin() + id * 2, uvLayers[i].begin() + id * 2 + 2);
        
        for(unsigned i = 0; i < rgbaLayers.size(); ++i)
            rgbaLayers[i].erase(rgbaLayers[i].begin() + id * 4, rgbaLayers[i].begin() + id * 4 + 4);
    }
    
    int64_t uid;

    CoordSystem coordSys;
    Axis right, up, front;
    
    std::string name;

    struct polygon
    {
        std::vector<int32_t> indices;
    };
    
    std::vector<int> indices;
    std::vector<polygon> polys;
    std::vector<float> vertices;
    std::vector<int32_t> origVertIndices;
    std::vector<std::vector<float>> normalLayers;
    std::vector<std::vector<float>> uvLayers;
    std::vector<std::vector<unsigned char>> rgbaLayers;
    
    Skin skin;
private:
    void _getVerticesAndIndices(Settings& settings, Node& rootNode, Node& geom, const Math::Mat4f& transform)
    {
        std::vector<int32_t> fbxIndices = 
            geom.Get("PolygonVertexIndex")[0].GetArray<int32_t>();
        std::vector<float> fbxVertices =
            geom.Get("Vertices")[0].GetArray<float>();

        for(unsigned i = 0; i < fbxVertices.size() / 3; ++i)
        {
            Au::Math::Vec3f v(
                fbxVertices[i * 3] * (float)settings.scaleFactor,
                fbxVertices[i * 3 + 1] * (float)settings.scaleFactor,
                fbxVertices[i * 3 + 2] * (float)settings.scaleFactor
            );
            v = transform * Au::Math::Vec4f(v.x, v.y, v.z, 1.0f);
            vertices.push_back(v.x);
            vertices.push_back(v.y);
            vertices.push_back(v.z);
        }

        int polySize = 0;
        for(unsigned i = 0; i < fbxIndices.size(); ++i)
        {
            int32_t fidx = fbxIndices[i];
            int32_t nfidx = fidx < 0 ? -fidx - 1 : fidx;
            polySize++;
            if(fidx < 0)
            {
                int32_t polyStart = i - polySize + 1;
                int32_t polyEnd = i;

                polygon poly;
                for(int32_t p = polyStart; p < polyEnd; ++p)
                    poly.indices.push_back(p);
                poly.indices.push_back(nfidx);
                polys.push_back(poly);
                
                for(int32_t p = polyStart; p <= polyEnd - 2; ++p)
                {
                    indices.push_back(fbxIndices[polyStart]);
                    indices.push_back(fbxIndices[p + 1]);
                    int32_t lastIdx = fbxIndices[p + 2] < 0 ? -fbxIndices[p + 2] - 1 : fbxIndices[p + 2];
                    indices.push_back(lastIdx);
                }

                polySize = 0;
            }
        }
        origVertIndices = indices;
    }
    
    void _getNormals(Settings& settings, Node& rootNode, Node& geom, const Math::Mat4f& transform)
    {
        std::vector<int32_t> fbxIndices = 
            geom.Get("PolygonVertexIndex")[0].GetArray<int32_t>();

        //normalLayers.resize(1);
        //normalLayers[0] = std::vector<float>(vertices.size());
        //return;
        
        int normalLayerCount = geom.Count("LayerElementNormal");
        for(int j = 0; j < normalLayerCount; ++j)
        {
            Node& layerElemNormal = geom.Get("LayerElementNormal", j);
            std::vector<float> fbxNormals = 
                layerElemNormal.Get("Normals")[0].GetArray<float>();
            std::vector<int32_t> fbxNormalsIndex =
                layerElemNormal.Get("NormalsIndex")[0].GetArray<int32_t>();
            std::string normalsMapping = 
                layerElemNormal.Get("MappingInformationType")[0].GetString();
            std::string refType =
                layerElemNormal.Get("ReferenceInformationType")[0].GetString();

            //std::cout << normalsMapping << ", " << refType << std::endl;
            
            std::vector<float> normals;
            
            if(normalsMapping == "ByVertex" || normalsMapping == "ByVertice")
            {
                normals.resize(fbxNormals.size());
                for(unsigned l = 0; l < fbxNormals.size() / 3; ++l)
                {
                    Au::Math::Vec3f norm(
                        fbxNormals[l * 3],
                        fbxNormals[l * 3 + 1],
                        fbxNormals[l * 3 + 2]
                    );
                    norm = transform * Au::Math::Vec4f(norm.x, norm.y, norm.z, 0.0f);
                    normals[l * 3] = norm.x;
                    normals[l * 3 + 1] = norm.y;
                    normals[l * 3 + 2] = norm.z;
                }
            }
            else if(normalsMapping == "ByPolygon")
            {
                normals = std::vector<float>(vertices.size());
                for(unsigned k = 0; k < polys.size(); ++k)
                {
                    Au::Math::Vec3f norm(
                        fbxNormals[k * 3],
                        fbxNormals[k * 3 + 1],
                        fbxNormals[k * 3 + 2]
                    );
                    norm = transform * Au::Math::Vec4f(norm.x, norm.y, norm.z, 0.0f);
                    for(unsigned l = 0; l < polys[k].indices.size(); ++l)
                    {
                        int32_t idx = polys[k].indices[l];
                        normals[idx * 3] = norm.x;
                        normals[idx * 3 + 1] = norm.y;
                        normals[idx * 3 + 2] = norm.z;
                    }
                }
            }
            else if(normalsMapping == "ByPolygonVertex")
            {
                normals = std::vector<float>(vertices.size());
                
                if(refType == "Direct")
                {
                    for(unsigned k = 0; k < fbxIndices.size(); ++k)
                    {
                        int32_t idx = fbxIndices[k];
                        idx = idx < 0 ? -idx - 1 : idx;
                        Au::Math::Vec3f norm(
                            fbxNormals[k * 3],
                            fbxNormals[k * 3 + 1],
                            fbxNormals[k * 3 + 2]
                        );
                        norm = transform * Au::Math::Vec4f(norm.x, norm.y, norm.z, 0.0f);
                        normals[idx * 3] = norm.x;
                        normals[idx * 3 + 1] = norm.y;
                        normals[idx * 3 + 2] = norm.z;
                    }
                }
                else if(refType == "IndexToDirect" || refType == "Index")
                {
                    for(unsigned k = 0; k < fbxIndices.size(); ++k)
                    {
                        int32_t idx = fbxNormalsIndex[k];
                        int32_t fidx = fbxIndices[k];
                        fidx = fidx < 0 ? -fidx - 1 : fidx;
                        Au::Math::Vec3f norm(
                            fbxNormals[k * 3],
                            fbxNormals[k * 3 + 1],
                            fbxNormals[k * 3 + 2]
                        );
                        norm = transform * Au::Math::Vec4f(norm.x, norm.y, norm.z, 0.0f);
                        normals[fidx * 3] = norm.x;
                        normals[fidx * 3 + 1] = norm.y;
                        normals[fidx * 3 + 2] = norm.z;
                    }
                } 
            }
            
            normalLayers.push_back(normals);
        }

        if(normalLayers.empty())
        {
            normalLayers.push_back(std::vector<float>(vertices.size()));
        }
    }
    
    void _getUV(Node& rootNode, Node& geom)
    {
        std::vector<int32_t> fbxIndices = 
            geom.Get("PolygonVertexIndex")[0].GetArray<int32_t>();

        //uvLayers.resize(1);
        //uvLayers[0] = std::vector<float>(vertices.size() / 3 * 2);
        //return;
            
        int layerCount = geom.Count("LayerElementUV");
        for(int i = 0; i < layerCount; ++i)
        {
            Node& layer = geom.Get("LayerElementUV", i);
            std::vector<float> fbxUV =
                layer.Get("UV")[0].GetArray<float>();
            std::vector<int32_t> fbxUVIndex =
                layer.Get("UVIndex")[0].GetArray<int32_t>();
            std::string mapping =
                layer.Get("MappingInformationType")[0].GetString();
            std::string refType =
                layer.Get("ReferenceInformationType")[0].GetString();
            
            std::vector<float> uv;
            
            if(mapping == "ByVertex" || mapping == "ByVertice")
            {
                for(unsigned l = 0; l < vertices.size() / 3; l += 3)
                {
                    uv.push_back(fbxUV[l / 3 * 2]);
                    uv.push_back(fbxUV[l / 3 * 2 + 1]);
                }
            }
            else if(mapping == "ByPolygon")
            {
                uv.resize(vertices.size() / 3 * 2);
                for(unsigned k = 0; k < polys.size(); ++k)
                {
                    Au::Math::Vec2f u(
                        fbxUV[k * 2],
                        fbxUV[k * 2 + 1]
                    );
                    for(unsigned l = 0; l < polys[k].indices.size(); ++l)
                    {
                        int32_t idx = polys[k].indices[l];
                        uv[idx * 2] = u.x;
                        uv[idx * 2 + 1] = u.y;
                    }
                }
            }
            else if(mapping == "ByPolygonVertex")
            {
                uv.resize(vertices.size() / 3 * 2);
                
                if(refType == "Direct")
                {
                    unsigned vertexCount = vertices.size() / 3;
                    for(unsigned k = 0; k < fbxIndices.size(); ++k)
                    {
                        int32_t idx = fbxIndices[k];
                        idx = idx < 0 ? -idx - 1 : idx;
                        Au::Math::Vec2f u(
                            fbxUV[k * 2],
                            fbxUV[k * 2 + 1]
                        );
                        uv[idx * 2] = u.x;
                        uv[idx * 2 + 1] = u.y;
                    }
                }
                else if(refType == "IndexToDirect" || refType == "Index")
                {
                    for(unsigned k = 0; k < fbxIndices.size(); ++k)
                    {
                        int32_t idx = fbxUVIndex[k];
                        int32_t fidx = fbxIndices[k];
                        fidx = fidx < 0 ? -fidx - 1 : fidx;
                        Au::Math::Vec2f u(
                            fbxUV[idx * 2],
                            fbxUV[idx * 2 + 1]
                        );
                        uv[fidx * 2] = u.x;
                        uv[fidx * 2 + 1] = u.y;
                    }
                } 
            }
            
            uvLayers.push_back(uv);
        }

        if(uvLayers.empty())
        {
            uvLayers.push_back(std::vector<float>(vertices.size() / 3 * 2));
        }
    }
};

}}}

#endif
