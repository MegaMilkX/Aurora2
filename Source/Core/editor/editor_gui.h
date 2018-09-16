#ifndef EDITOR_GUI_H
#define EDITOR_GUI_H

#include "editor_scene_hierarchy.h"
#include "editor_scene_object_inspector.h"
#include "editor_component_creator.h"
#include "../external/imguizmo/imguizmo.h"
#include <camera.h>
#include "../data_headers/blender_icons.png.h"
#include "../input.h"
#include "../external/nativefiledialog/nfd.h"

#include <global_objects.h>
#include <external/scene_from_fbx.h>

#include <components/animation_driver.h>

#include "editor_config.h"

class EditorCamera : public SceneObject::Component
{
    RTTR_ENABLE(SceneObject::Component)
public:
    LightDirect* lightDirect;
    void OnInit()
    {
        c = Get<Camera>();
        c->Get<Transform>()->Translate(0.0f, 0.0f, 5.0f);

        c->Perspective(1.0f, 16.0f/9.0f, 0.1f, 100.0f);
        auto lo = c->Get<LightOmni>();
        lo->Color(1.0f, 1.0f, 1.0f);
        lo->Intensity(1.0f);

        lightDirect = c->Get<LightDirect>();
        lightDirect->Direction(gfxm::vec3(0.0f, -1.0f, 0.0f));
        lightDirect->Color(gfxm::vec3(1.0f, 1.0f, 1.0f));

        camPivot = CreateObject()->Get<Transform>();
        camPivot->Attach(c->Get<Transform>());

        gInput.BindActionPress(
            "MouseLookHold",
            [this](){
                mouse_look = true;
            }
        );
        gInput.BindActionRelease(
            "MouseLookHold",
            [this](){
                mouse_look = false;
            }
        );

        gInput.BindActionPress(
            "MouseLookAlt",
            [this](){ mouse_look_alt = true; }
        );
        gInput.BindActionRelease(
            "MouseLookAlt",
            [this](){ mouse_look_alt = false; }
        );

        gInput.BindAxis(
            "MoveCamX",
            [this](float v){
                if(mouse_look && mouse_look_alt){
                    camPivot->Get<Transform>()->Rotate(-v * 0.01f, gfxm::vec3(0.0f, 1.0f, 0.0f));
                } else if(mouse_look){
                    float mod = c->Get<Transform>()->Position().z;
                    gfxm::vec3 right = camPivot->Get<Transform>()->Right();
                    camPivot->Get<Transform>()->Translate(-right * v * 0.01f * mod * 0.15f);
                }
            }
        );
        gInput.BindAxis(
            "MoveCamY",
            [this](float v){
                if(mouse_look && mouse_look_alt){
                    camPivot->Get<Transform>()->Rotate(-v * 0.01f, camPivot->Get<Transform>()->Right());
                } else if(mouse_look){
                    float mod = c->Get<Transform>()->Position().z;
                    gfxm::vec3 up = camPivot->Get<Transform>()->Up();
                    camPivot->Get<Transform>()->Translate(up * v * 0.01f * mod * 0.15f);
                }
            }
        );

        gInput.BindAxis(
            "CameraZoom",
            [this](float v){
                float mod = c->Get<Transform>()->Position().z;
                c->Get<Transform>()->Translate(gfxm::vec3(0.0f, 0.0f, -v * mod * 0.15f));
                gfxm::vec3 pos = c->Get<Transform>()->Position();
                if(pos.z < 0.0f)
                {
                    c->Get<Transform>()->Position(gfxm::vec3(pos.x, pos.y, 0.0f));
                }
            }
        );
    }

    void Reset(const gfxm::vec3& tgt)
    {
        camPivot->Position(tgt);
        gfxm::vec3 pos = c->Get<Transform>()->Position();
        c->Get<Transform>()->Position(gfxm::vec3(pos.x, pos.y, 3.0f));
    }
private:
    bool mouse_look = false;
    bool mouse_look_alt = false;
    Camera* c;
    Transform* camPivot;
};
STATIC_RUN(EditorCamera)
{
    rttr::registration::class_<EditorCamera>("EditorCamera");
}

class EditorGui
{
public:
    EditorGui()
    : selectedObject(0) 
    {
        editorSceneHierarchy.SetSelectCallback(std::bind(&EditorGui::OnObjectSelect, this, std::placeholders::_1));
    }

    enum TRANSFORM_GIZMO_STATE
    {
        TRANSLATE,
        ROTATE,
        SCALE
    };
    TRANSFORM_GIZMO_STATE gizmoState = TRANSLATE;

    std::shared_ptr<Texture2D> icon_texture;
    SceneObject editorScene;
    SceneObject* editedScene;
    EditorCamera* editorCamera = 0;

    void Init(Input* input)
    {
        char* outPath;
        auto r = NFD_OpenDialog(NULL, NULL, &outPath);

        editorCamera = editorScene.CreateObject()->Get<EditorCamera>();
        editedScene = editorScene.CreateObject();
        editedScene->Name("Root");

        icon_texture.reset(new Texture2D());
        std::shared_ptr<DataSource> raw(
            new DataSourceMemory((char*)blender_icons_png, sizeof(blender_icons_png))
        );
        icon_texture->Build(raw);

        editorSceneObjectInspector.AddComponentGuiExtension(
            rttr::type::get<AnimationDriver>(),
            [](SceneObject::Component* c){
                AnimationDriver* driver = (AnimationDriver*)c;
                bool preview = true;
                if(ImGui::Checkbox("Preview", &preview)) {}
                if(preview) glfwPostEmptyEvent();

                for(size_t i = 0; i < driver->LayerCount(); ++i)
                {
                    AnimLayer* layer = driver->GetLayer(i);
                    if (ImGui::TreeNode(MKSTR("Layer " << i).c_str()))
                    {
                        if (ImGui::BeginCombo("Current anim", layer->CurrentAnimName().c_str(), 0)) // The second parameter is the label previewed before opening the combo.
                        {
                            for(size_t n = 0; n < driver->AnimCount(); ++n) {
                                const std::string& anim_name = driver->GetAnimName(n);
                                bool is_selected = (anim_name == layer->CurrentAnimName());
                                if(ImGui::Selectable(anim_name.c_str(), is_selected)) {
                                    layer->Play(anim_name);
                                }
                                if(is_selected) ImGui::SetItemDefaultFocus();
                            }
                            ImGui::EndCombo();
                        }
                        rttr::type t = rttr::type::get<AnimLayer::MODE>();
                        rttr::enumeration en = t.get_enumeration();
                        if(ImGui::BeginCombo("Mode", en.value_to_name(layer->Mode()).to_string().c_str()))
                        {                            
                            for(auto& n : en.get_names()) {
                                auto& val = en.name_to_value(n);
                                bool is_selected = (val.get_value<AnimLayer::MODE>() == layer->Mode());
                                if(ImGui::Selectable(n.to_string().c_str(), is_selected)) {
                                    layer->SetMode(val.get_value<AnimLayer::MODE>());
                                }
                                if(is_selected) ImGui::SetItemDefaultFocus();
                            }
                            ImGui::EndCombo();
                        }
                        float weight = layer->Strength();
                        if(ImGui::DragFloat("Weight", &weight, 0.01f, 0.0f, 1.0f)) {
                            layer->SetStrength(weight);
                        }
                        bool looping = layer->Looping();
                        if(ImGui::Checkbox("Looping", &looping)) {}
                        layer->Looping(looping);
                        if(ImGui::Button("Remove Layer")) {
                            driver->RemoveLayer(i);
                        }
                        ImGui::TreePop();
                    }
                }
                if(ImGui::Button("Add Layer")) {
                    driver->AddLayer();
                }
                if (ImGui::TreeNode("Animations"))
                {
                    for(size_t i = 0; i < driver->AnimCount(); ++i)
                    {
                        const std::string& name = driver->GetAnimName(i);
                        ImGui::Text(name.c_str());
                    }
                    ImGui::TreePop();
                }
            }
        );

        input->BindActionPress(
            "GizmoTranslate", 
            [this](){
                gizmoState = TRANSLATE;
            }
        );
        input->BindActionPress(
            "GizmoRotate", 
            [this](){
                gizmoState = ROTATE;
            }
        );
        input->BindActionPress(
            "GizmoScale", 
            [this](){
                gizmoState = SCALE;
            }
        );
        gInput.BindActionPress(
            "ResetEditorCam",
            [this](){
                gfxm::vec3 resetPos(0.0f, 0.0f, 0.0f);
                if(selectedObject)
                {
                    resetPos = selectedObject->Get<Transform>()->WorldPosition();
                }
                editorCamera->Reset(resetPos);
            }
        );
    }

    SceneObject* GetScene() { return &editorScene; }

    void OnObjectSelect(SceneObject* o)
    {
        selectedObject = o;
    }

    void Draw()
    {
        Camera* camera = editorCamera->Get<Camera>();
        glDisable(GL_CULL_FACE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);

        editorCamera->lightDirect->Direction(editorCamera->Get<Transform>()->Forward());

        if(camera)
        {
            if(selectedObject && selectedObject->FindComponent<Transform>())
            {
                ImGuizmo::SetRect(0, 0, Common.frameSize.x, Common.frameSize.y);
                gfxm::mat4 proj = camera->Projection();
                gfxm::mat4 view = gfxm::inverse(camera->Get<Transform>()->GetTransform());
                gfxm::mat4 model = selectedObject->Get<Transform>()->GetTransform();
                gfxm::mat4 dModel(1.0f);

                if(gizmoState == TRANSLATE)
                {
                    ImGuizmo::Manipulate((float*)&view, (float*)&proj, ImGuizmo::OPERATION::TRANSLATE,ImGuizmo::MODE::LOCAL, (float*)&model, (float*)&dModel, 0);
                    if(ImGuizmo::IsUsing())
                    {
                        gfxm::vec4 dT = dModel[3];
                        selectedObject->Get<Transform>()->Translate(dT);
                    }
                }
                else if(gizmoState == ROTATE)
                {
                    ImGuizmo::Manipulate((float*)&view, (float*)&proj, ImGuizmo::OPERATION::ROTATE,ImGuizmo::MODE::LOCAL, (float*)&model, (float*)&dModel, 0);
                    if(ImGuizmo::IsUsing())
                    {
                        gfxm::mat3 deltaRotM3 = gfxm::to_mat3(dModel);
                        gfxm::quat deltaRot = gfxm::to_quat(deltaRotM3);
                        selectedObject->Get<Transform>()->Rotate(deltaRot);
                    }
                }
                else if(gizmoState == SCALE)
                {
                    ImGuizmo::Manipulate((float*)&view, (float*)&proj, ImGuizmo::OPERATION::SCALE,ImGuizmo::MODE::LOCAL, (float*)&model, (float*)&dModel, 0);
                    if(ImGuizmo::IsUsing())
                    {
                        gfxm::vec3 right = model[0];
                        gfxm::vec3 up = model[1];
                        gfxm::vec3 back = model[2];
                        gfxm::vec3 dScale = gfxm::vec3(
                            right.length(), 
                            up.length(), 
                            back.length()
                        );
                        selectedObject->Get<Transform>()->Scale(dScale);
                    }
                }
            }
        }
        
        if(ImGui::BeginMainMenuBar())
        {
            if(ImGui::BeginMenu("File"))
            {
                if(ImGui::MenuItem("New")) 
                {
                    editorScene.Erase(editedScene);
                    editedScene = editorScene.CreateObject();
                    editedScene->Name("Root");
                    editorCamera->Reset(gfxm::vec3(0.0f,0.0f,0.0f));
                    currentSceneFile.clear();
                    selectedObject = 0;
                }
                if(ImGui::MenuItem("Open")) 
                {
                    char* outPath;
                    auto r = NFD_OpenDialog("scn", NULL, &outPath);
                    if(r == NFD_OKAY) {
                        editorScene.Erase(editedScene);
                        editedScene = editorScene.CreateObject();
                        editedScene->Name("Root");

                        std::cout << outPath << std::endl;
                        std::string filePath(outPath);
                        GlobalSceneSerializer().Deserialize(filePath, *editedScene);
                        editorCamera->Reset(gfxm::vec3(0.0f,0.0f,0.0f));
                        currentSceneFile = filePath;
                        selectedObject = 0;
                    }
                    else if(r == NFD_CANCEL) {
                        std::cout << "cancelled" << std::endl;
                    }
                    else {
                        std::cout << "error " << NFD_GetError() << std::endl;
                    }
                }
                if(ImGui::MenuItem("Import FBX..."))
                {
                    char* outPath;
                    auto r = NFD_OpenDialog("fbx", NULL, &outPath);
                    if(r == NFD_OKAY) {
                        editorScene.Erase(editedScene);
                        editedScene = editorScene.CreateObject();
                        editedScene->Name("Root");

                        std::cout << outPath << std::endl;
                        std::string filePath(outPath);
                        SceneFromFbx(filePath, editedScene);
                        editorCamera->Reset(gfxm::vec3(0.0f,0.0f,0.0f));
                        currentSceneFile.clear();
                        selectedObject = 0;
                    }
                }
                if(ImGui::MenuItem("Merge..."))
                {
                    char* outPath;
                    auto r = NFD_OpenDialog("scn", NULL, &outPath);
                    if(r == NFD_OKAY) {
                        auto so = editedScene->CreateObject();

                        std::cout << outPath << std::endl;
                        std::string filePath(outPath);
                        GlobalSceneSerializer().Deserialize(filePath, *so);
                    }
                }
                if(ImGui::MenuItem("Merge FBX..."))
                {
                    char* outPath;
                    auto r = NFD_OpenDialog("fbx", NULL, &outPath);
                    if(r == NFD_OKAY) {
                        auto so = editedScene->CreateObject();

                        std::cout << outPath << std::endl;
                        std::string filePath(outPath);
                        SceneFromFbx(filePath, so);
                    }
                }
                if(ImGui::MenuItem("Save"))
                {
                    if(currentSceneFile.empty())
                    {
                        char* outPath;
                        auto r = NFD_SaveDialog("scn", NULL, &outPath);
                        if(r == NFD_OKAY) {
                            std::cout << outPath << std::endl;
                            std::string filePath(outPath);
                            if(GlobalSceneSerializer().Serialize(editedScene, filePath))
                            {
                                std::cout << "Scene saved" << std::endl;
                                currentSceneFile = filePath;
                            }
                        }
                    }
                    else
                    {
                        if(GlobalSceneSerializer().Serialize(editedScene, currentSceneFile))
                        {
                            std::cout << "Scene saved" << std::endl;
                        }
                    }
                }
                if(ImGui::MenuItem("Save As..."))
                {
                    char* outPath;
                    auto r = NFD_SaveDialog("scn", NULL, &outPath);
                    if(r == NFD_OKAY) {
                        std::cout << outPath << std::endl;
                        std::string filePath(outPath);
                        GlobalSceneSerializer().Serialize(editedScene, filePath);
                    }
                }
                if(ImGui::MenuItem("Exit")) {}
                ImGui::EndMenu();
            }
            if(ImGui::BeginMenu("Build"))
            {
                if(ImGui::MenuItem("Build")) {}
                if(ImGui::MenuItem("Build and run")) 
                {
                    if(!EditorConfig().projectConfPath.empty()
                        && !EditorConfig().builderPath.empty())
                    {
                        if(system(("call \"" + EditorConfig().builderPath + "\" \"" + EditorConfig().projectConfPath + "\"").c_str()) != 0)
                        {
                            std::cout << "Failed to build project" << std::endl;
                            return;
                        }
                    }
                }
                if(ImGui::MenuItem("Run")) {}
                ImGui::Separator();
                if(ImGui::MenuItem("Rebuild resources")) {}
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        bool t = true;
        if(ImGui::Begin("Toolbar", &t))
        {
            ImTextureID my_tex_id = (ImTextureID)icon_texture.get();
            float my_tex_w = (float)icon_texture->Width();
            float my_tex_h = (float)icon_texture->Height();
            if (ImGui::ImageButton(my_tex_id, ImVec2(32,32), ImVec2(0,0), ImVec2(32.0f/(float)my_tex_w,32/(float)my_tex_h), 1, ImColor(0,0,0,255)))
            {}
            ImGui::SameLine();
            if (ImGui::ImageButton(my_tex_id, ImVec2(22,22), ImVec2(4.0f/my_tex_w,(my_tex_h - 155.0f)/my_tex_h), ImVec2(22.0f/my_tex_w,(my_tex_h - 174.0f)/my_tex_h), 1, ImColor(0,0,0,255)))
            {}
            ImGui::SameLine();
            if (ImGui::ImageButton(my_tex_id, ImVec2(32,32), ImVec2(0,0), ImVec2(32.0f/(float)my_tex_w,32/(float)my_tex_h), 1, ImColor(0,0,0,255)))
            {}

            ImGui::NewLine();
            ImGui::End();
        }
        editorSceneHierarchy.Draw(editedScene);
        editorSceneObjectInspector.Draw(selectedObject, componentCreator);
        componentCreator.Update(selectedObject);

        if(ImGui::Begin("Resource Inspector", &t)) {
            for(size_t i = 0; i < GlobalDataRegistry().Count(); ++i) {
                ImGui::Text(GlobalDataRegistry().GetNameById(i).c_str());
            }

            ImGui::End();
        }
    }
    
private:
    EditorSceneHierarchy editorSceneHierarchy;
    EditorSceneObjectInspector editorSceneObjectInspector;
    EditorComponentCreator componentCreator;

    SceneObject* selectedObject;
    std::string currentSceneFile;
};

#endif
