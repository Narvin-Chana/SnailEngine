#include "stdafx.h"
#include "InstancedEntity.h"

#include "Core/WindowsEngine.h"

namespace Snail
{
InstancedEntity::Params::Params()
{
    name = "Instanced Entity";
}

InstancedEntity::InstancedEntity(const Params& params)
    : Entity(params)
    , instanceTransforms{params.instanceTransforms}
{
#ifdef _IMGUI_
    hiddenInstances.resize(instanceTransforms.size());
#endif
    shouldFrustumCull = false;
}

void InstancedEntity::Draw(DrawContext& ctx)
{
    if (!mesh)
        return;

    const DirectX::BoundingBox baseBoundingBox = GetBoundingBox();

    for (int i = 0; i < instanceTransforms.size(); ++i)
    {
        DirectX::BoundingBox instanceBoundingBox;
        // Multiply by entity transform for relative position to instanced entity
        Matrix modelMatrix = instanceTransforms[i].GetTransformationMatrix();
        baseBoundingBox.Transform(instanceBoundingBox, modelMatrix);

#ifdef _IMGUI_
        if (hiddenInstances[i])
            continue;
#endif

        if (ctx.ShouldBeCulled(instanceBoundingBox))
            continue;

        mesh->SubscribeInstance(modelMatrix);
    }
}

void InstancedEntity::RenderImGui(int idNumber)
{
    UNREFERENCED_PARAMETER(idNumber);
#ifdef _IMGUI_
    Entity::RenderImGui(idNumber);

    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
    if (ImGui::TreeNodeEx(("Instances ##" + std::to_string(idNumber) + entityName).c_str()))
    {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));

        std::string key = std::to_string(idNumber) + entityName;

        auto logInstances = [&]()
            {
                nlohmann::json j = nlohmann::json::array();
                for (auto& instanceTransform : instanceTransforms)
                {
                    j.push_back(instanceTransform);
                }
                LOG(j);
            };

        if (ImGui::Button(("Export instance transforms##" + key).c_str()))
        {
            logInstances();
        }

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
        for (int i = 0; i < instanceTransforms.size(); ++i)
        {
            if (ImGui::TreeNodeEx(("Instance " + std::to_string(i)).c_str()))
            {
                ImGui::PushID(i);

                if (ImGui::Button(("Duplicate ##" + key).c_str()))
                {
                    logInstances();
                    instanceTransforms.push_back(instanceTransforms.empty() ? Transform{} : instanceTransforms.back());
                    hiddenInstances.push_back(false);
                }

                bool b = hiddenInstances[i];
                ImGui::Checkbox("Visible", &b);
                hiddenInstances[i] = b;
                instanceTransforms[i].RenderImGui();

                ImGui::PopID();
                ImGui::TreePop();
            }
        }

        ImGui::PopStyleColor();

        ImGui::PopStyleColor();

        ImGui::TreePop();
    }
    ImGui::PopStyleColor();
#endif
}

std::string InstancedEntity::GetJsonType()
{
    return "instanced_entity";
}
}
