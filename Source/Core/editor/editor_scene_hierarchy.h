#ifndef EDITOR_SCENE_HIERARCHY_H
#define EDITOR_SCENE_HIERARCHY_H

#include <component.h>
#include <util/imgui_wrapper.h>
#include <util/imgui_console.h>
#include <functional>

class EditorSceneHierarchy
{
public:
    typedef std::function<void(SceneObject*)> object_select_cb_t;

    void SetSelectCallback(object_select_cb_t cb) { objectSelectCb = cb; }

    void Draw(SceneObject* scene)
    {
        bool t = true;
        const float DISTANCE = 10.0f;
        const float DISTANCE_Y = 30.0f;
        int corner = 0;
        ImVec2 window_pos = ImVec2((corner & 1) ? ImGui::GetIO().DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? ImGui::GetIO().DisplaySize.y - DISTANCE_Y : DISTANCE_Y);
        ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        if (!ImGui::Begin("Scene Hierarchy", &t, 0))
        {
            // Early out if the window is collapsed, as an optimization.
            ImGui::End();
        }
        else
        {
            if(ImGui::Button("Create Object")) {
                SceneObject* o = scene->CreateObject();
                SetSelected(o);
            }
            ImGui::Separator();
            SceneHierarchyViewNode(scene);
            ImGui::End();
        }
    }

private:
    void SceneHierarchyViewNode(SceneObject* scene)
    {
        if(scene->ChildCount() == 0)
        {
            if (ImGui::Selectable(MKSTR(scene->Name() << "##" << scene).c_str(), selectedObject == scene ? true : false))
            {
                SetSelected(scene);
            }
            if (ImGui::BeginPopupContextItem()) // When used after an item that has an ID (here the Button), we can skip providing an ID to BeginPopupContextItem().
            {
                ImGui::Selectable("Export...");
                if(ImGui::Selectable("Delete")) {
                    if(scene->Parent()) {
                        scene->Parent()->Erase(scene);
                    }
                }
                ImGui::EndPopup();
            }
        }
        else
        {
            bool node_open = ImGui::TreeNodeEx(
                (void*)scene, selectedObject == scene ? ImGuiTreeNodeFlags_Selected : 0, MKSTR(scene->Name()).c_str());
            if (ImGui::BeginPopupContextItem()) // When used after an item that has an ID (here the Button), we can skip providing an ID to BeginPopupContextItem().
            {
                ImGui::Selectable("Export...");
                ImGui::Selectable("Delete");
                ImGui::EndPopup();
            }

            if (ImGui::IsItemClicked())
            {
                SetSelected(scene);
            }
            if (node_open)
            {
                for(unsigned i = 0; i < scene->ChildCount(); ++i)
                {
                    SceneHierarchyViewNode(scene->GetChild(i));
                }
                ImGui::TreePop();
            }
        }
    }

    void SetSelected(SceneObject* o)
    {
        selectedObject = o;
        if(objectSelectCb) objectSelectCb(o);
    }
    SceneObject* selectedObject = 0;
    object_select_cb_t objectSelectCb;
};

#endif
