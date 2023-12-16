#include "stdafx.h"

#include "Entity.h"

#include "Core/WindowsEngine.h"
#include "Core/Physics/DynamicPhysicsObject.h"

namespace Snail
{

Entity::Entity(const Params& params) noexcept
    : entityName{params.name}
    , mesh{params.mesh}
    , transform{params.transform}
    , physicsObject{params.physicsObject}
    , castsShadows{params.castsShadows}
{}

Entity::Entity(Entity&&) noexcept = default;

Entity& Entity::operator=(Entity&&) noexcept = default;

Entity::~Entity() = default;

void Entity::Update(const float) noexcept
{
    if (physicsObject)
    {
        dirtyFlags.set();
        physicsObject->UpdateTransform(transform);
    }
}

void Entity::Draw(DrawContext& ctx)
{
    if (!mesh || (shouldFrustumCull && ctx.ShouldBeCulled(GetBoundingBox())))
        return;

    mesh->SubscribeInstance(GetWorldTransformMatrix());
}

const Transform& Entity::GetTransform() const { return transform; }

void Entity::SetTransform(const Transform& t)
{
    dirtyFlags.set();

    if (!physicsObject)
    {
        transform = t;
        return;
    }

    if (physicsObject->SetTransform(t)) { transform = t; }
}

void Entity::SetPosition(const Vector3& pos)
{
    dirtyFlags.set();

    if (!physicsObject)
    {
        transform.position = pos;
        return;
    }

    Transform newTransform = transform;
    newTransform.position = pos;
    if (physicsObject->SetTransform(newTransform)) { transform.position = pos; }
}

Entity* Entity::GetParent() const noexcept { return parent; }

void Entity::InitPhysics()
{
    if (!physicsObject)
        return;

    physicsObject->SetUserData(userData.get());
}

void Entity::PrepareShadows()
{
    static D3D11Device* device = WindowsEngine::GetInstance().GetRenderDevice();
    device->SetBackFaceCulling();
}

void Entity::SetRotation(const Vector3& euler)
{
    dirtyFlags.set();
    transform.rotation = Quaternion::CreateFromYawPitchRoll(euler);
    if (physicsObject) { physicsObject->SetTransform(transform); }
}

void Entity::SetRotation(const Quaternion& quat)
{
    dirtyFlags.set();
    transform.rotation = quat;
    if (physicsObject) { physicsObject->SetTransform(transform); }
}

void Entity::SetScale(const Vector3& scale)
{
    dirtyFlags.set();
    transform.scale = scale;
}

void Entity::SetParent(Entity* pParent) noexcept
{
    dirtyFlags.set();
    parent = pParent;
}

Transform Entity::GetWorldTransform()
{
    if (!parent)
    {
        return GetTransform();
    }

    if (!dirtyFlags.test(2))
    {
        return worldTransform;
    }

    Matrix worldMatrix = GetWorldTransformMatrix();
    worldMatrix.Decompose(worldTransform.scale, worldTransform.rotation, worldTransform.position);
    dirtyFlags.set(2, false);
    return worldTransform;
}

const BaseMesh* Entity::GetMesh() const noexcept { return mesh; }

Vector3 Entity::GetBoundsLocalCenter() const noexcept
{
    if (!mesh) { return Vector3::Zero; }

    auto [min, max] = mesh->GetBounds();
    return (min + max) * 0.5f;
}

const DirectX::BoundingBox& Entity::GetBoundingBox()
{
    static RendererModule& renderer = WindowsEngine::GetModule<RendererModule>();
    const Transform _worldTransform = GetWorldTransform();

    if (!dirtyFlags.test(1))
    {
#ifdef _DEBUG
        if (renderer.drawEntityTransform)
        {
            // Additional computations only occur for debugging purposes
            const Vector3 globalCenter = Vector3::Transform(GetBoundsLocalCenter(), GetWorldTransformMatrix());
            const Vector3 extents = GetExtents() * _worldTransform.scale;

            // Scaled orientation
            const Vector3 right = _worldTransform.GetRightVector() * extents.x;
            const Vector3 up = _worldTransform.GetUpVector() * extents.y;
            const Vector3 forward = _worldTransform.GetForwardVector() * extents.z;

            renderer.DrawLine({globalCenter, Color(1, 0, 0)}, {globalCenter + right * _worldTransform.scale * 2.0f, Color(1, 0, 0)});
            renderer.DrawLine({globalCenter, Color(0, 1, 0)}, {globalCenter + up * _worldTransform.scale * 2.0f, Color(0, 1, 0)});
            renderer.DrawLine({globalCenter, Color(0, 0, 1)}, {globalCenter + forward * _worldTransform.scale * 2.0f, Color(0, 0, 1)});
        }
#endif
        return boundingBox;
    }

    // ===================================================================================================
    // Have to recalculate bounds here with transformed points if we want to get a more accurate AABB.
    // Current solution prioritises speed over precision of AABB.
    // ===================================================================================================

    // Get global scale thanks to our transform
    const Vector3 globalCenter = Vector3::Transform(GetBoundsLocalCenter(), GetWorldTransformMatrix());

    const Vector3 extents = GetExtents() * _worldTransform.scale;

    // Scaled orientation
    const Vector3 right = _worldTransform.GetRightVector() * extents.x;
    const Vector3 up = _worldTransform.GetUpVector() * extents.y;
    const Vector3 forward = _worldTransform.GetForwardVector() * extents.z;

    // Readjust extent based on global position of entity
    const float newIi = std::abs(Vector3::UnitX.Dot(right)) + std::abs(Vector3::UnitX.Dot(up)) + std::abs(Vector3::UnitX.Dot(forward));
    const float newIj = std::abs(Vector3::UnitY.Dot(right)) + std::abs(Vector3::UnitY.Dot(up)) + std::abs(Vector3::UnitY.Dot(forward));
    const float newIk = std::abs(Vector3::UnitZ.Dot(right)) + std::abs(Vector3::UnitZ.Dot(up)) + std::abs(Vector3::UnitZ.Dot(forward));

#ifdef _DEBUG
    if (renderer.drawEntityTransform)
    {
        renderer.DrawLine({globalCenter, Color(1, 0, 0)}, {globalCenter + right * _worldTransform.scale * 2.0f, Color(1, 0, 0)});
        renderer.DrawLine({globalCenter, Color(0, 1, 0)}, {globalCenter + up * _worldTransform.scale * 2.0f, Color(0, 1, 0)});
        renderer.DrawLine({globalCenter, Color(0, 0, 1)}, {globalCenter + forward * _worldTransform.scale * 2.0f, Color(0, 0, 1)});
    }
#endif

    // Store center and extents.
    boundingBox = DirectX::BoundingBox{globalCenter, Vector3{newIi, newIj, newIk}};
    dirtyFlags.set(1, false);

    return boundingBox;
}

const PhysicsObject* Entity::GetPhysicsObject() const noexcept { return physicsObject.get(); }

Matrix Entity::GetWorldTransformMatrix()
{
    if (!dirtyFlags.test(0)) { return worldTransformMatrix; }

    // Update world transform matrix
    if (!parent) { worldTransformMatrix = transform.GetTransformationMatrix(); }
    else
    {
        const Matrix parentTransform = parent->transform.GetTransformationMatrix();
        worldTransformMatrix = transform.GetTransformationMatrix() * parentTransform;
    }

    dirtyFlags.set(0, false);

    return worldTransformMatrix;
}

bool Entity::ShouldCastShadows() const noexcept { return castsShadows; }

Vector3 Entity::GetExtents() const noexcept
{
    if (!mesh) { return Vector3::Zero; }
    auto [min, max] = mesh->GetBounds();
    return (max - min) * 0.5f;
}

bool Entity::ShouldFrustumCull() const noexcept { return shouldFrustumCull; }

std::string Entity::GetJsonType()
{
    return "entity";
}

#ifdef _IMGUI_

void Entity::RenderImGui(const int idNumber)
{
    ImGui::BeginGroup();
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
    if (ImGui::TreeNodeEx(("General Settings ##" + std::to_string(idNumber) + entityName).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));

        bool castShadowsPreviously = castsShadows;
        ImGui::Text("Casts Shadows: ");
        ImGui::SameLine();
        ImGui::Checkbox(("##" + std::to_string(idNumber) + entityName + "Cast Shadows Toggle").c_str(), &castShadowsPreviously);

        castsShadows = castShadowsPreviously;

        if (physicsObject.get()) { physicsObject->RenderImGui(); }
        else { ImGui::Text("Physics Type: None"); }

        ImGui::PopStyleColor();

        ImGui::TreePop();
    }
    if (mesh && ImGui::TreeNodeEx(("Mesh ##" + std::to_string(idNumber) + entityName).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
        mesh->RenderImGui();
        ImGui::PopStyleColor();
        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx(("Transform ##" + std::to_string(idNumber) + entityName).c_str()))
    {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));

        if (transform.RenderImGui())
        {
            SetTransform(transform);
        }
        
        ImGui::PopStyleColor();

        ImGui::TreePop();
    }

    if (ImGui::SmallButton(std::format("Export##{}", std::to_string(idNumber)).c_str()))
    {
        nlohmann::json j;
        j["transform"] = transform;
        j["type"] = GetJsonType();
        j["name"] = entityName;
        LOGF("Exported entity: {}", j.dump());
    }

    ImGui::PopStyleColor();
    ImGui::EndGroup();
}
#else
void Entity::RenderImGui(int) {}
#endif
}
