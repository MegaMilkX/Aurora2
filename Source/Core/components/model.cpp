#include "model.h"

Model::Model()
: dirty(true),
transform(0),
renderer(0)
{
}

Model::~Model()
{
    
}

void Model::OnInit()
{
    static std::once_flag once_flag;
    std::call_once(
        once_flag,
        [this](){
            resource<gl::ShaderProgram> prog = 
                resource<gl::ShaderProgram>::get("solid_shader");
            gl::Shader vs;
            gl::Shader fs;
            vs.Init(GL_VERTEX_SHADER);
            vs.Source(
                #include "../shaders/solid_vs.glsl"
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

            prog->BindFragData(0, "fragOut");
            prog->Link();

            prog->Use();
            glUniform1i(prog->GetUniform("DiffuseTexture"), 0);
        }
    );

    program = resource<gl::ShaderProgram>::get("solid_shader");
    SolidDrawData sdd{ 
        program,
        program->GetUniform("MatrixProjection"),
        program->GetUniform("MatrixView"),
        program->GetUniform("MatrixModel"),
        program->GetUniform("AmbientColor")
    };
    renderer->GetFrameGraph().set_data(sdd);

    transform = GetObject()->GetComponent<Transform>();
    renderer = GetObject()->Root()->GetComponent<Renderer>();
    
    task_graph::graph& fg = renderer->GetFrameGraph();
    fg += fg_SolidDraw;
    fg += task_graph::once(fg_SolidRebuild);
    fg.reset_once_flag(fg_SolidRebuild);
}
std::string Model::Serialize() 
{
    using json = nlohmann::json;
    json j = json::object();
    j["Material"] = materialName;
    j["Mesh"] = meshName;
    j["SubMesh"] = subMeshName;
    return j.dump(); 
}
void Model::Deserialize(const std::string& data) 
{
    using json = nlohmann::json;
    json j = json::parse(data);
    if(j.is_null())
        return;
    if(j["Mesh"].is_string())
    {
        mesh.set(j["Mesh"].get<std::string>());
    }
    if(j["SubMesh"].is_string())
    {
        mesh.set(j["SubMesh"].get<std::string>());
    }
    if(j["Material"].is_string())
    {
        mesh.set(j["Material"].get<std::string>());
    }
}
