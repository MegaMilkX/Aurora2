#ifndef GENERIC_UTIL_H
#define GENERIC_UTIL_H

#include <aurora/media/fbx.h>

Au::GFX::Mesh* LoadMesh(Au::GFX::Device* gfxDevice, const std::string& path)
{
    Au::GFX::Mesh* mesh = gfxDevice->CreateMesh();
    mesh->Format(Au::Position() << 
                Au::Normal() << 
                Au::ColorRGB() << 
                Au::BoneWeight4() << 
                Au::BoneIndex4());
    
    // FBX Loading WIP =========================
    
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> buffer((unsigned int)size);
    if(file.read(buffer.data(), size))
    {
        Au::Media::FBX::Reader fbxReader;
        fbxReader.ReadMemory(buffer.data(), buffer.size());
        fbxReader.ConvertCoordSys(Au::Media::FBX::OPENGL);
        //fbxReader.Print();
        
        int meshCount = fbxReader.MeshCount();
        if(meshCount > 0)
        {
            Au::Media::FBX::Mesh& fbxMesh = fbxReader.GetMesh(0);
            std::vector<float> vertices = fbxMesh.GetVertices();
            mesh->VertexAttrib<Au::Position>(vertices);
            std::vector<float> normals = fbxMesh.GetNormals(0);
            mesh->VertexAttrib<Au::Normal>(normals);
            std::vector<unsigned short> indices = fbxMesh.GetIndices<unsigned short>();
            mesh->IndexData(indices);
            

            //std::vector<int> boneIndices;
            //std::vector<unsigned char> boneWeights;
            //std::vector<Au::Media::FBX::Deformer> deformers = fbxMesh.GetDeformers();
        }
    }
    // =========================================
    
    return mesh;
}

Au::GFX::RenderState* CreateRenderState(Au::GFX::Device* gfxDevice)
{
    Au::GFX::Shader* shaderVertex = gfxDevice->CreateShader(Au::GFX::Shader::VERTEX);
    shaderVertex->Source(R"(#version 130
        uniform mat4 MatrixModel;
        uniform mat4 MatrixView;
        uniform mat4 MatrixProjection;
        in vec3 Position;
        in vec3 Normal;
        in vec3 ColorRGB;
        out vec3 color;
        out vec3 normal;
        
        out vec3 fragPos;
        
        uniform mat4 Bones[32];
        in vec4 BoneWeight4;
        in ivec4 BoneIndex4;
        
        void main()
        {
            vec3 skinnedPos = (BoneWeight4[0] * (Bones[BoneIndex4[0]] * vec4(Position, 1.0)) + 
                              BoneWeight4[1] * (Bones[BoneIndex4[1]] * vec4(Position, 1.0)) + 
                              BoneWeight4[2] * (Bones[BoneIndex4[2]] * vec4(Position, 1.0)) + 
                              BoneWeight4[3] * (Bones[BoneIndex4[3]] * vec4(Position, 1.0))
                              ).xyz;
            vec3 skinnedNormal = BoneWeight4[0] * (mat3(Bones[BoneIndex4[0]]) * Normal) +
                                 BoneWeight4[1] * (mat3(Bones[BoneIndex4[1]]) * Normal) +
                                 BoneWeight4[2] * (mat3(Bones[BoneIndex4[2]]) * Normal) +
                                 BoneWeight4[3] * (mat3(Bones[BoneIndex4[3]]) * Normal);
            
            skinnedNormal = normalize(skinnedNormal);
            
            fragPos = vec3(MatrixModel * vec4(Position, 1.0));
            color = ColorRGB;
            normal = (MatrixModel * vec4(Normal, 0.0)).xyz;
            gl_Position = MatrixProjection * MatrixView * MatrixModel * vec4(Position, 1.0);
    })");
    std::cout << shaderVertex->StatusString() << std::endl;
    
    Au::GFX::Shader* shaderPixel = gfxDevice->CreateShader(Au::GFX::Shader::PIXEL);
    shaderPixel->Source(R"(#version 130
        in vec3 color;
        in vec3 normal;
        in vec3 fragPos;
        
        uniform vec3 LightOmniPos[3];
        uniform vec3 LightOmniRGB[3];
        
        out vec4 fragColor;
        
        void main()
        {
            vec3 ambient_color = vec3(0.2, 0.2, 0.2);            
            vec3 result = ambient_color;
            
            for(int i = 0; i < 3; i++)
            {
                vec3 lightDir = normalize(LightOmniPos[i] - fragPos);
                float diff = max(dot(normal, lightDir), 0.0);
                float dist = distance(LightOmniPos[i], fragPos);
                vec3 diffuse = LightOmniRGB[i] * diff * (1.0 / (1.0 + 0.5 * dist + 3.0 * dist * dist));
                
                result += diffuse;
            }
            
            fragColor = vec4(result, 1.0);
    })");
    std::cout << shaderPixel->StatusString() << std::endl;
    
    Au::GFX::RenderState* renderState = gfxDevice->CreateRenderState();
    renderState->AttribFormat(Au::Position() << Au::Normal() << Au::ColorRGB());
    renderState->SetShader(shaderVertex);
    renderState->SetShader(shaderPixel);
    renderState->AddUniform<gfxm::mat4>("MatrixModel");
    renderState->AddUniform<gfxm::mat4>("MatrixView");
    renderState->AddUniform<gfxm::mat4>("MatrixProjection");
    renderState->AddUniform<gfxm::vec3>("LightOmniPos", 3);
    renderState->AddUniform<gfxm::vec3>("LightOmniRGB", 3);
    renderState->AddUniform<gfxm::mat4>("Bones", 32);
    
    std::cout << renderState->StatusString() << std::endl;
    
    return renderState;
}

#endif
