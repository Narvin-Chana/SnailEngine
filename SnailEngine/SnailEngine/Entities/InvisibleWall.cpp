#include "stdafx.h"
#include "InvisibleWall.h"

#include "Core/WindowsEngine.h"
#include "Core/Physics/StaticPhysicsObject.h"

namespace Snail
{
InvisibleWall::InvisibleWall(const Params& params)
    : Entity{params}
{
#ifdef _DEBUG // Show boxes in debug
    static auto& mm = WindowsEngine::GetModule<MeshManager>();
    mesh = mm.GetAsset<BaseMesh>("Cube");
    castsShadows = false;
#endif
}

void InvisibleWall::InitPhysics()
{
    static PhysicsModule& physModule = WindowsEngine::GetModule<PhysicsModule>();
    PhysXUniquePtr<PxShape> shape;
    shape.reset(physModule.physics->createShape(PxBoxGeometry{ transform.scale.x / 2, transform.scale.y / 2, transform.scale.z / 2 }, *physModule.defaultMaterial));
    physicsObject = std::make_unique<StaticPhysicsObject>(shape.get(), transform);
}

std::string InvisibleWall::GetJsonType()
{
    return "invisible_wall";
}

void InvisibleWall::RenderImGui(int idNumber)
{
    UNREFERENCED_PARAMETER(idNumber);
#ifdef _IMGUI_
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
    if (ImGui::TreeNodeEx(("Invisible Wall Settings ##" + std::to_string(idNumber) + entityName).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));

        ImGui::Checkbox(("Show Colliders##Show collider " + std::to_string(idNumber)).c_str(), &showColliders);

        ImGui::PopStyleColor();
        ImGui::TreePop();
    }
    ImGui::PopStyleColor();
    Entity::RenderImGui(idNumber);
#endif
}

void InvisibleWall::Draw(DrawContext& ctx)
{
#ifdef _IMGUI_
    if (showColliders)
#endif
    Entity::Draw(ctx);
}
}
