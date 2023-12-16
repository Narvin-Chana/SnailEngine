#include "stdafx.h"
#include "Billboard.h"

#include "Core/WindowsEngine.h"
#include "Core/Mesh/QuadMesh.h"

namespace Snail
{
Billboard::Params::Params()
{
    static MeshManager& meshManager = WindowsEngine::GetModule<MeshManager>();
    name = "Billboard";
    mesh = meshManager.GetAsset<QuadMesh>("Quad");
    billboardType = WORLD_ALIGNED;
}

Billboard::Billboard(const Params& params)
    : Entity(params)
    , billboardTransformMatrixBuffer(D3D11Buffer::CreateConstantBuffer<Matrix>())
    , type{params.billboardType}
{
    // Don't perform frustum culling on billboards since their final position depends on the view camera and not on their world position
    shouldFrustumCull = false;
    // Casting shadows on billboards is weird.
    castsShadows = false;
}

Matrix Billboard::GetWorldTransformMatrix()
{
    // Flags have no purpose here since the camera almost always is moving
    // So update the matrix each frame
    const Camera* cam = WindowsEngine::GetCamera();

    const Transform worldTr = GetWorldTransform();
    const Transform camTransform = cam->GetTransform();
    
    const Matrix viewProjMatrix = cam->GetViewProjectionMatrix();
    const Matrix scale = Matrix::CreateScale(worldTr.scale);

    switch (type)
    {
        case WORLD_ALIGNED:
        {
            // Apply transformation using camera to make the billboard always face the camera's position
            const Matrix rotateTranslate = Matrix::CreateLookAt(worldTr.position, camTransform.position, camTransform.GetUpVector()).Invert();
            worldTransformMatrix = scale * rotateTranslate * viewProjMatrix;
            break;
        }
        case SCREEN_ALIGNED:
        {
            // Apply transformation using camera to make the billboard always face the camera's frustum plane
            const Matrix rotateTranslate = Matrix::CreateLookAt(worldTr.position, worldTr.position - camTransform.GetForwardVector(), camTransform.GetUpVector()).Invert();
            worldTransformMatrix = scale * rotateTranslate * viewProjMatrix;
            break;
        }
        case AXIAL_ALIGNED:
        {
            // Apply transformation using camera to make the billboard always face the camera's position while rotating only on the y axis
            const Matrix rotate{ -camTransform.GetRightVector(), Vector3::UnitY, worldTr.GetForwardVector() };
            const Matrix translate = Matrix::CreateTranslation(worldTr.position);
            worldTransformMatrix = scale * rotate * translate * viewProjMatrix;
            break;
        }
        default:
            LOG(Logger::ERROR, "Billboard ", entityName, " has invalid billboard type: ", type);
            throw InvalidBillBoardTypeException{};
    }

    return worldTransformMatrix;
}

void Billboard::Draw(DrawContext&)
{
    // TODO: improve this by adding translucency and rendering all non-opaque objects after deferred
    mesh->SubscribeInstance(GetWorldTransformMatrix());
}

void Billboard::RenderImGui(const int idNumber)
{
    UNREFERENCED_PARAMETER(idNumber);
#ifdef _IMGUI_
    ImGui::Text("Billboard Type: ");
    static constexpr std::array possibleTypes = {"World-Oriented", "Screen-Aligned", "Axial-Aligned",};
    if (ImGui::BeginCombo("##billboard type", possibleTypes[static_cast<int>(type)]))
    {
        for (int n = 0; n < possibleTypes.size(); n++)
        {
            const bool isSelected = (type == n);
            if (ImGui::Selectable(possibleTypes[n], isSelected))
            {
                type = static_cast<BillboardType>(n);
            }
            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

    // Add toggling billboard type
    Entity::RenderImGui(idNumber);
#endif
}

std::string Billboard::GetJsonType()
{
    return "billboard";
}
}
