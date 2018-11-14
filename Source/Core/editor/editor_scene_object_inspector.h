#ifndef EDITOR_SCENE_OBJECT_INSPECTOR_H
#define EDITOR_SCENE_OBJECT_INSPECTOR_H

#include <component.h>
#include <util/imgui_wrapper.h>
#include <util/imgui_console.h>
#include "editor_component_creator.h"
#include "editor_data_pick.h"

#include "editor_gui_property.h"

class EditorSceneObjectInspector
{
public:
    void AddComponentGuiExtension(rttr::type component_type, std::function<void(Component*)> func) {
        componentGuiExtensions[component_type] = func;
    }

    void Draw(SceneObject* object, EditorComponentCreator& componentCreatorWindow)
    {
        //dataPick.Update();

        bool t = true;
        const float DISTANCE = 10.0f;
        const float DISTANCE_Y = 30.0f;
        int corner = 1;
        ImVec2 window_pos = ImVec2((corner & 1) ? ImGui::GetIO().DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? ImGui::GetIO().DisplaySize.y - DISTANCE_Y : DISTANCE_Y);
        ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        if(!ImGui::Begin("SceneObject Inspector", &t, 0))
        {
            ImGui::End();
        }
        else
        {
            DrawComponents(object, componentCreatorWindow);
        }
    }

private:
    std::map<rttr::type, std::function<void(Component*)>> componentGuiExtensions;
    //EditorDataPick dataPick;

    void ImGuiDrawProperties(void* instance, rttr::type type) {
        for(auto prop : type.get_properties()) {
            Editor::GUI::Property::Draw(prop, instance);
        }
    }

    void DrawProperties(SceneObject* object, Component* comp) {
        rttr::type type = comp->GetType();
        //ImGuiDrawProperties((void*)comp, type);
        for(auto prop : type.get_properties())
        {
            rttr::type ptype = prop.get_type();
            //ImGui::Text(prop.get_name().to_string().c_str());
            
            if(ptype == rttr::type::get<gfxm::vec3>())
            {
                gfxm::vec3 val = prop.get_value(comp).get_value<gfxm::vec3>();
                if(ImGui::DragFloat3(prop.get_name().to_string().c_str(), (float*)&val, 0.001f))
                {
                    prop.set_value(comp, val);
                }
            }
            else if(ptype == rttr::type::get<float>())
            {
                float val = prop.get_value(comp).get_value<float>();
                if(ImGui::DragFloat(prop.get_name().to_string().c_str(), &val, 0.001f))
                {
                    prop.set_value(comp, val);
                }
            }
            else if(ptype.is_derived_from<i_resource_ref>()) {
                rttr::variant var = prop.get_value(comp);
                i_resource_ref& ref = var.get_value<i_resource_ref>();
                ImGui::Text(prop.get_name().to_string().c_str());
                ImVec4 buttonColor = ImColor(0.2f, 0.2f, 0.7f, 1.0f);
                std::string res_name;
                if(ref.base_ptr()) {
                    res_name = ref.base_ptr()->Name();
                    if(ref.base_ptr()->Storage() == Resource::LOCAL) {
                        buttonColor = ImColor(0.7f, 0.2f, 0.2f, 1.0f);
                    }
                }
                if(res_name.empty()) res_name = "[NULL]";

                ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
                //ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(i/7.0f, 0.7f, 0.7f));
                //ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(i/7.0f, 0.8f, 0.8f));
                if(ImGui::Button(res_name.c_str())) {
                    //dataPick.Show();
                    //dataPick.SetTarget(prop, comp);
                }
                ImGui::PopStyleColor(1);
            }
            else if(ptype == rttr::type::get<std::weak_ptr<SceneObject>>()) {
                ImGui::Text(prop.get_name().to_string().c_str());
                rttr::variant var = prop.get_value(comp);
                std::weak_ptr<SceneObject> weak_so = var.get_value<std::weak_ptr<SceneObject>>();
                auto so = weak_so.lock();
                std::string tgt_name = "[NULL]";
                if(so) {
                    tgt_name = so->Name();
                }
                if(ImGui::Button(tgt_name.c_str())) {
                    // TODO: Open scene object picker
                }
            }
            /* TODO
            else if(ptype == rttr::type::get<ResourceRef>())
            {
                ImGui::Text(prop.get_name().to_string().c_str());
                std::string res_name;// = prop.get_value(comp).get_value<ResourceRef>().GetTargetName();
                if(res_name.empty()) res_name = "[empty]";
                if(ImGui::Button(res_name.c_str())){

                }
            }*/
        }
    }

    void DrawComponents(SceneObject* object, EditorComponentCreator& componentCreatorWindow) {
        if(!object)
            ImGui::Text("No object selected");
        else
        {
            std::string buf = object->Name();
            buf.resize(256);
            if(ImGui::InputText("Name", (char*)buf.data(), buf.size()))
            {
                object->Name(buf);
            }
            ImGui::Text(MKSTR("UID: " << object->Uid()).c_str());
            if(ImGui::Button("Add component...")) {
                componentCreatorWindow.Show();
            }
            ImGui::Separator();
            for(unsigned i = 0; i < object->ComponentCount(); ++i)
            {
                Component* comp = object->GetComponent(i);
                rttr::type type = comp->GetType();

                ImGui::Text(type.get_name().to_string().c_str());
                if(!comp->_editor()) {
                    ImGui::Text("[no gui]");
                }

                /*
                if(componentGuiExtensions.count(type) > 0)
                {
                    componentGuiExtensions[type](comp);
                }
                DrawProperties(object, comp);
                */
            }
        }
        ImGui::End();
    }
};

#endif
