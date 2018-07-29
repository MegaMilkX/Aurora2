#ifndef SKELETON_H
#define SKELETON_H

#include "renderer.h"
#include "model.h"
#include <component.h>

#include <algorithm>
#include <mutex>
#include "../util/gl_render_state.h"
#include <util/scoped_timer.h>

#undef GetObject

struct BoneData
{
    BoneData(const std::string& name, gfxm::mat4 transform, const std::string& parentName, bool isBone)
    : name(name), transform(transform), parentName(parentName), isBone(isBone)
    {}
    ~BoneData()
    { for(unsigned i = 0; i < children.size(); ++i) delete children[i]; }
    
    std::string name;
    std::string parentName;
    bool isBone;
    gfxm::mat4 transform;
    std::vector<BoneData*> children;
};

struct SkeletonData
{
    SkeletonData()
    : boneCount(0)
    {}
    ~SkeletonData()
    { for(unsigned i = 0; i < rootBones.size(); ++i) delete rootBones[i]; }
    void AddNode(const std::string& name, gfxm::mat4 transform, const std::string& parentName, bool isBone)
    {
        BoneData* bd = new BoneData(name, transform, parentName, isBone);
        rootBones.push_back(bd);
        boneCount++;
    }
    void Finalize()
    {
        std::vector<BoneData*>& bones = 
            this->rootBones;
        std::vector<BoneData*> rootBones;
        std::vector<unsigned> rootBoneIndices;
        for(unsigned i = 0; i < bones.size(); ++i)
        {
            BoneData* parent = GetBoneData(bones[i]->parentName);
            if(parent)
                parent->children.push_back(bones[i]);
            else
                rootBoneIndices.push_back(i);
        }
        
        for(unsigned i = 0; i < rootBoneIndices.size(); ++i)
            rootBones.push_back(bones[rootBoneIndices[i]]);
        bones = rootBones;
    }
    int BoneCount() { return boneCount; }
    BoneData* GetBoneData(const std::string& name)
    {
        std::vector<BoneData*>& bones = 
            this->rootBones;
        for(unsigned i = 0; i < bones.size(); ++i)
        {
            if(bones[i]->name == name)
                return bones[i];
        }
        return 0;
    }

    bool Build(Resource* r)
    {
        if(!r) return false;

        for(auto kv : r->GetChildren())
        {
            Resource* res_name = kv.second->Get("Name");
            Resource* res_transform = kv.second->Get("Transform");
            Resource* res_parent = kv.second->Get("Parent");
            Resource* res_is_bone = kv.second->Get("IsBone");
            if(!res_name || !res_transform) return false;
            std::string name((char*)res_name->Data(), (char*)res_name->Data() + res_name->DataSize());
            std::string parent((char*)res_parent->Data(), (char*)res_parent->Data() + res_parent->DataSize());
            gfxm::mat4 transform = *(gfxm::mat4*)res_transform->Data();
            bool is_bone = *(bool*)res_is_bone->Data();
            if(res_parent) parent = std::string((char*)res_parent->Data(), (char*)res_parent->Data() + res_parent->DataSize());
            AddNode(name, transform, parent, is_bone);
        }

        Finalize();

        return true;
    }

    std::vector<BoneData*> rootBones;
    int boneCount;
};

class Skeleton;
struct RenderUnitSkin
{    
    GLuint vao;
    int indexCount;
    asset<Texture2D> texDiffuse;

    int vertexSize;
    Transform* transform;
    Skeleton* skeleton;
};

struct SkinDrawData
{
    resource<gl::ShaderProgram> program;
    GLuint uniProjection;
    GLuint uniView;
    GLuint uniModel;
    GLuint uniAmbientColor;
    GLuint uniBoneInverseTransforms;
    GLuint uniBoneTransforms; 
    std::vector<RenderUnitSkin> units;
};

inline void fg_SkinRebuild(const FrameCommon& frame, SkinDrawData& out);
inline void fg_SkinDraw(const FrameCommon& frame, const SkinDrawData& out);

class Skeleton : public SceneObject::Component
{
public:
    Skeleton()
    {
        skinShaderSource =
            R"(
            #vertex MatrixSkin
                in vec4 BoneIndex4;
                in vec4 BoneWeight4;
                uniform mat4 BoneInverseBindTransforms[MAX_BONE_COUNT];
                uniform mat4 BoneTransforms[MAX_BONE_COUNT];
                out mat4 MatrixSkin;
                
                int bi0 = int(BoneIndex4.x);
                int bi1 = int(BoneIndex4.y);
                int bi2 = int(BoneIndex4.z);
                int bi3 = int(BoneIndex4.w);
                MatrixSkin = 
                    BoneTransforms[bi0] * BoneInverseBindTransforms[bi0] * BoneWeight4.x +
                    BoneTransforms[bi1] * BoneInverseBindTransforms[bi1] * BoneWeight4.y +
                    BoneTransforms[bi2] * BoneInverseBindTransforms[bi2] * BoneWeight4.z +
                    BoneTransforms[bi3] * BoneInverseBindTransforms[bi3] * BoneWeight4.w;
                
            #vertex PositionModel
                in vec3 Position;
                in mat4 MatrixSkin;
                out vec4 PositionModel;
                
                PositionModel =
                    MatrixSkin *
                    vec4(Position, 1.0);
                    
            #vertex NormalModel
                in vec3 Normal;
                in mat4 MatrixSkin;
                uniform mat4 MatrixModel;
                out vec3 NormalModel;
                NormalModel = normalize((MatrixModel * MatrixSkin * vec4(Normal, 0.0)).xyz);
            )";
    }
    
    ~Skeleton()
    {
        
    }
    
    void SetData(const std::string& name)
    {
        resourceName = name;
        SetData(asset<SkeletonData>::get(name));
    }
    
    void SetData(SkeletonData* data)
    {
        skelData = data;
        for(unsigned i = 0; i < skelData->rootBones.size(); ++i)
        {
            BoneData* bone = skelData->rootBones[i];
            CreateBone(bone, GetObject());
        }
        
        SortAndFinalizeBoneArrays();
        /*
        int maxBoneCount = renderer->GetInt("MAX_BONE_COUNT");
        if((int)bones.size() > maxBoneCount)
            renderer->SetInt("MAX_BONE_COUNT", bones.size());
        */
    }
    
    void Bind(GLuint shaderProgram)
    {/*
        for(unsigned i = 0; i < bones.size(); ++i)
        {
            uniformBoneInverseBinds[i] = boneInverseBindTransforms[i];
            uniformBoneTransforms[i] = boneTransforms[i];
        }
        */
        GLuint loc = glGetUniformLocation(shaderProgram, "BoneInverseBindTransforms[0]");
        glUniformMatrix4fv(
            loc, 
            (std::min)((unsigned)32, (unsigned)boneInverseBindTransforms.size()), 
            GL_FALSE, 
            (GLfloat*)boneInverseBindTransforms.data()
        );
        
        loc = glGetUniformLocation(shaderProgram, "BoneTransforms[0]");
        glUniformMatrix4fv(
            loc, 
            (std::min)((unsigned)32, (unsigned)boneTransforms.size()), 
            GL_FALSE, 
            (GLfloat*)boneTransforms.data()
        );
    }
    
    void Update()
    {
        for(unsigned i = 0; i < bones.size(); ++i)
            boneTransforms[i] = 
                gfxm::inverse(transform->GetTransform()) *
                bones[i]->GetTransform();
    }

    virtual void OnInit()
    {
        transform = Get<Transform>();
        renderer = GetObject()->Root()->GetComponent<Renderer>();
        
        static std::once_flag once_flag;
        std::call_once(
            once_flag,
            [this](){
                resource<gl::ShaderProgram> prog = 
                    resource<gl::ShaderProgram>::get("skin_shader");
                gl::Shader vs;
                gl::Shader fs;
                vs.Init(GL_VERTEX_SHADER);
                vs.Source(
                    #include "../shaders/skin_vs.glsl"
                );
                vs.Compile();
                fs.Init(GL_FRAGMENT_SHADER);
                fs.Source(
                    #include "../shaders/solid_fs.glsl"
                );
                fs.Compile();

                prog->AttachShader(&vs);
                prog->AttachShader(&fs);

                prog->BindAttrib(0, "Position");
                prog->BindAttrib(1, "UV");
                prog->BindAttrib(2, "Normal");
                prog->BindAttrib(3, "BoneIndex4");
                prog->BindAttrib(4, "BoneWeight4");

                prog->BindFragData(0, "fragOut");
                prog->Link();

                prog->Use();
                glUniform1i(prog->GetUniform("DiffuseTexture"), 0);

                SkinDrawData sdd{ 
                    prog,
                    prog->GetUniform("MatrixProjection"),
                    prog->GetUniform("MatrixView"),
                    prog->GetUniform("MatrixModel"),
                    prog->GetUniform("AmbientColor"),
                    prog->GetUniform("BoneInverseBindTransforms"),
                    prog->GetUniform("BoneTransforms")
                };
                renderer->GetFrameGraph().set_data(sdd);
                
            }
        );

        Model* m = GetComponent<Model>();
        if(m)
            m->program = resource<gl::ShaderProgram>::get("skin_shader");

        task_graph::graph& fg = renderer->GetFrameGraph();
        fg += task_graph::once(fg_SkinRebuild);
        fg.reset_once_flag(fg_SkinRebuild);
        fg += fg_SkinDraw;
    }
    virtual std::string Serialize() 
    { 
        using json = nlohmann::json;
        json j = json::object();
        j["Data"] = resourceName;
        return j.dump(); 
    }
    virtual void Deserialize(const std::string& data)
    {
        using json = nlohmann::json;
        json j = json::parse(data);
        if(j.is_null())
            return;
        if(j["Data"].is_string())
        {
            SetData(j["Data"].get<std::string>());
        }
    }
private:
    void CreateBone(BoneData* bone, SceneObject* parentObject)
    {
        SceneObject* boneObject = parentObject->FindObject(bone->name);
        if(!boneObject)
        {
            boneObject = parentObject->CreateObject();
            boneObject->Name(bone->name);
        }
        
        //std::cout << boneObject->Name() << std::endl;
        
        Transform* boneTransform = boneObject->GetComponent<Transform>();
        
        if(bone->isBone)
        {
            bones.push_back(boneTransform);
        }
        
        boneTransform->SetTransform(bone->transform);
        boneTransform->AttachTo(parentObject->GetComponent<Transform>());
        
        for(unsigned i = 0; i < bone->children.size(); ++i)
        {
            BoneData* boneData = bone->children[i];
            CreateBone(boneData, boneObject);
        }
    }
    
    static bool CompBoneTransforms(Transform*& first, Transform*& second)
    {
        return first->GetObject()->Name() < second->GetObject()->Name();
    }
    
    void SortAndFinalizeBoneArrays()
    {
        std::sort(bones.begin(), bones.end(), &CompBoneTransforms);
        boneInverseBindTransforms.resize(bones.size());
        boneTransforms.resize(bones.size());
        for(unsigned i = 0; i < bones.size(); ++i)
        {
            boneInverseBindTransforms[i] = 
                gfxm::inverse(
                    gfxm::inverse(GetObject()->GetComponent<Transform>()->GetTransform()) *
                    bones[i]->GetTransform()
                );
        }
        
        Update();
    }

    asset<SkeletonData> skelData;
    std::string resourceName;
    
    Transform* transform;
    Renderer* renderer;
    
    std::vector<Transform*> bones;
    std::vector<gfxm::mat4> boneInverseBindTransforms;
    std::vector<gfxm::mat4> boneTransforms;
    
    std::string skinShaderSource;
};
COMPONENT(Skeleton)

inline void fg_SkinRebuild(const FrameCommon& frame, SkinDrawData& out)
{
    Renderer* renderer = frame.scene->GetComponent<Renderer>();
    std::vector<Model*> models = frame.scene->FindAllOf<Model>();
    for(Model* model : models)
    {
        if(!model->GetObject()->FindComponent<Skeleton>())
            continue;
        if(!model->program.equals(out.program))
            continue;
        if(model->mesh.empty())
            continue;
        RenderUnitSkin unit;
        unit.skeleton = model->GetComponent<Skeleton>();
        unit.transform = model->GetComponent<Transform>();
        unit.vao = model->mesh->GetVao({
            { "Position", 3, GL_FLOAT, GL_FALSE },
            { "UV", 2, GL_FLOAT, GL_FALSE },
            { "Normal", 3, GL_FLOAT, GL_FALSE },
            { "BoneIndex4", 4, GL_FLOAT, GL_FALSE },
            { "BoneWeight4", 4, GL_FLOAT, GL_FALSE }
        });
        unit.indexCount = model->mesh->GetIndexCount();
        unit.texDiffuse.set(model->material->GetString("Diffuse"));

        out.units.push_back(unit);
    }

    std::cout << "Created " << out.units.size() << " skin units" << std::endl;
}
inline void fg_SkinDraw(const FrameCommon& frame, const SkinDrawData& in)
{
    in.program->Use();
    glUniformMatrix4fv(
        in.uniProjection, 1, GL_FALSE,
        (float*)&frame.projection
    );
    glUniformMatrix4fv(
        in.uniView, 1, GL_FALSE,
        (float*)&frame.view
    );
    glUniform3f(
        in.program->GetUniform("ViewPos"),
        frame.viewPos.x,
        frame.viewPos.y,
        frame.viewPos.z
    );
    glUniform3f(
        in.uniAmbientColor,
        0.1f, 0.15f, 0.25f
    );

    std::vector<LightDirect*> lds = frame.scene->FindAllOf<LightDirect>();
    for(unsigned i = 0; i < lds.size(); ++i)
    {
        auto l = lds[i];
        glUniform3f(
            in.program->GetUniform("LightDirect[" + std::to_string(i) + "]"), 
            l->Direction().x,
            l->Direction().y,
            l->Direction().z
        );
        glUniform3f(
            in.program->GetUniform("LightDirectRGB[" + std::to_string(i) + "]"), 
            l->Color().x,
            l->Color().y,
            l->Color().z
        );
    }
    std::vector<LightOmni*> los = frame.scene->FindAllOf<LightOmni>();
    for(unsigned i = 0; i < los.size(); ++i)
    {
        auto l = los[i];
        glUniform3f(
            in.program->GetUniform("LightOmniPos[" + std::to_string(i) + "]"), 
            l->Get<Transform>()->WorldPosition().x,
            l->Get<Transform>()->WorldPosition().y,
            l->Get<Transform>()->WorldPosition().z
        );
        glUniform3f(
            in.program->GetUniform("LightOmniRGB[" + std::to_string(i) + "]"), 
            l->Color().x,
            l->Color().y,
            l->Color().z
        );
    }

    for(const RenderUnitSkin& unit : in.units)
    {
        unit.skeleton->Update();
        unit.skeleton->Bind(in.program->GetId());
        glUniformMatrix4fv(
            in.uniModel, 1, GL_FALSE,
            (float*)&unit.transform->GetTransform()
        );
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, unit.texDiffuse->GetGlName());
        
        glBindVertexArray(unit.vao);
        glDrawElements(
            GL_TRIANGLES, 
            unit.indexCount, 
            GL_UNSIGNED_INT, 
            (void*)0
        );
    }
}

#endif
