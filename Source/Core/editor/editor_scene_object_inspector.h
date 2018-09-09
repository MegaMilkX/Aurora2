#ifndef EDITOR_SCENE_OBJECT_INSPECTOR_H
#define EDITOR_SCENE_OBJECT_INSPECTOR_H

#include <scene_object.h>
#include <util/imgui_wrapper.h>
#include <util/imgui_console.h>
#include "editor_component_creator.h"

class EditorSceneObjectInspector
{
public:
    void AddComponentGuiExtension(rttr::type component_type, std::function<void(SceneObject::Component*)> func) {
        componentGuiExtensions[component_type] = func;
    }

    void Draw(SceneObject* object, EditorComponentCreator& componentCreatorWindow)
    {
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
                if(ImGui::Button("Add component...")) {
                    componentCreatorWindow.Show();
                }
                ImGui::Separator();
                for(unsigned i = 0; i < object->ComponentCount(); ++i)
                {
                    SceneObject::Component* comp = object->GetComponent(i);
                    rttr::type type = comp->GetType();
                    if (ImGui::TreeNode(type.get_name().to_string().c_str()))
                    {
                        if(componentGuiExtensions.count(type) > 0)
                        {
                            componentGuiExtensions[type](comp);
                        }

                        for(auto prop : type.get_properties())
                        {
                            rttr::type ptype = prop.get_type();
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
                            else if(ptype == rttr::type::get<ResourceRef>())
                            {
                                ImGui::Text(prop.get_name().to_string().c_str());
                                std::string res_name;// = prop.get_value(comp).get_value<ResourceRef>().GetTargetName();
                                if(res_name.empty()) res_name = "[empty]";
                                if(ImGui::Button(res_name.c_str())){

                                }
                            }
                        }
                        ImGui::TreePop();
                    }
                }
            }
            ImGui::End();
        }
    }

private:
    std::map<rttr::type, std::function<void(SceneObject::Component*)>> componentGuiExtensions;
};

#endif
