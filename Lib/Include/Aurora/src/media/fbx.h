#ifndef FBX_H
#define FBX_H

#include <sstream>
#include <fstream>

#include "fbxmodel.h"
#include "fbxobject.h"
#include "fbxmesh.h"
#include "fbxanimationstack.h"
#include "fbxbone.h"

#include "fbxsettings.h"

#include "../timer.h"

namespace Au{
namespace Media{
namespace FBX{

typedef uint8_t Byte;
typedef uint32_t Word;
typedef uint64_t DoubleWord;

class Reader
{
public:
    Reader()
    {}
    bool ReadMemory(const char* data, unsigned size);
    
    void ConvertCoordSys(CoordSystem sys);
    
    Axis GetUpAxis();
    Axis GetFrontAxis();
    Axis GetRightAxis();
    
    TIME_MODE GetTimeMode();
    int GetTimeProtocol();
    double GetCustomFrameRate();
    double GetFrameRate();
    
    std::vector<AnimationStack>& GetAnimationStacks();
    
    void FlipAxis(Axis& axis);
    
    std::vector<Bone>& GetBones();
    Bone* GetBoneByUID(int64_t uid)
    {
        for(unsigned i = 0; i < bones.size(); ++i)
        {
            if(bones[i].deformerUID == uid)
                return &bones[i];
        }
        return 0;
    }
    
    unsigned ModelCount();
    Model* GetModel(unsigned id);
    Model* GetModelByUID(int64_t uid);
    
    int MeshCount() { return meshes.size(); }
    Mesh& GetMesh(unsigned id) { return meshes[id]; }
    Mesh* GetMesh(const std::string& name)
    {
        for(unsigned i = 0; i < meshes.size(); ++i)
            if(meshes[i].name == name)
                return &meshes[i];
        return 0;
    }
    
    Node& GetRootNode() { return rootNode; }
    
    void Print() 
    {
        std::ostringstream sstr;
        rootNode.Print(sstr);
        std::cout << sstr.str();
    }
    void DumpFile(const std::string& filename)
    {
        std::ostringstream sstr;
        rootNode.Print(sstr);
        
        std::ofstream file(filename + ".dump", std::ios::out);
        file << sstr.str();
        file.close();
    }
private:
    void ReadData(Prop& prop, std::vector<char>& out, const char* data, const char*& cursor, const char* end);
    bool ReadBlock(Node& node, const char* data, const char*& cursor, const char* end, Word flags);
    bool ReadFileFBX(const char* data, unsigned size);
    
    bool ReadVerticesAndIndices(Mesh& mesh, unsigned meshId);
    bool ReadNormals(Mesh& mesh, unsigned meshId);
    bool ReadUV(Mesh& mesh, unsigned meshId);
    bool ReadWeights(Mesh& mesh, unsigned meshId);
    bool ReadSkin(Mesh& mesh, unsigned meshId);
    
    void _loadModels();
    void _loadBones();
    
    std::vector<Node> GetConnectedChildren(const std::string& childName, Node& node);
    
    Axis FBXAxisToAxis(unsigned axis);
    
    Mesh* GetBoneDeformTarget(Node* bone);
    
    Settings settings;
    Node rootNode;
    CoordSystem coordSys;
    std::vector<Model> models;
    std::vector<Mesh> meshes;
    std::vector<Bone> bones;
    std::vector<AnimationStack> animStacks;
};

}
}
}

#endif
