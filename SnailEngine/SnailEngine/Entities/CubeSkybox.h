#pragma once
#include "Core/WindowsEngine.h"
#include "Entities/Entity.h"

namespace Snail
{
class DrawContext;

class CubeSkybox : public Entity
{
    Camera* targetCamera = WindowsEngine::GetInstance().GetCamera();

public:
    struct Params : Entity::Params
    {
        Params();
    };

    CubeSkybox(const Params& params);
    void Draw(DrawContext& ctx) override;
};
}
